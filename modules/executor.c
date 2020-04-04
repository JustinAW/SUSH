/************************************************
 *       Shippensburg University Shell          *
 *                 executor.c                   *
 ************************************************
 * executor takes a linked list and runs the    *
 * strings stored in it as a command            *
 ************************************************
 * Author: Justin Weigle                        *
 * Edited: 04 Apr 2020                          *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "../includes/sush.h"
#include "../includes/tokenizer.h"
#include "../includes/executor.h"

typedef struct path_node {
    char *path;
    struct path_node *next;
} path_node;

struct subsection {
    tok_node *head;
    tok_node *tail;
    int count;
};

enum Read_Write {
    READ,
    WRITE
};

static void count_tl (tok_node *, int *, int *);
static void output_to_file (char *, bool);
static void file_to_input (char *);
static void save_path(char *, path_node **);
static path_node* get_path();
static void free_path (path_node *);
static bool bin_exists (char *);
static int get_fd (char *, enum Read_Write, bool);
static struct subsection get_next_subsection (tok_node *);

/** gets a file descriptor for the given file name f */
static int get_fd (char *f, enum Read_Write rw, bool append)
{
    int fd;
    if (rw == READ) {
        fd = open(f, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
    }
    if (rw == WRITE) {
        if (append) {
            fd = open(f, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);
        } else {
            fd = open(f, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
        }
    }
    if (fd < 0) {
        perror("open failed in get_fd\n");
        exit(-1);
    }

    return fd;
}

/** redirect stdout to the file listed after >
 *  append controls whether the file is appended or overwritten */
static void output_to_file (char *fname, bool append)
{
    int fd;
    fd = get_fd(fname, WRITE, append);
    dup2(fd, STDOUT_FILENO); // stdout > file
    close(fd); // done, connection made with dup2
}

/** redirect a file to stdin */
static void file_to_input (char *fname)
{
    int fd;
    fd = get_fd(fname, READ, false);
    dup2(fd, STDIN_FILENO); // stdin < file
    close(fd); // done, connection made with dup2
}

static void parse_cmd (struct subsection cmd_ll)
{
    char *cmd[cmd_ll.count +1];

    tok_node *curr = cmd_ll.head;
    int i = 0;

    /* build command. don't include redirects */
    while ((curr != NULL)  && !(curr->special)) {
        cmd[i++] = curr->token;
        curr = curr->next;
    }
    cmd[i] = NULL;

    curr = cmd_ll.head;
    while (curr != cmd_ll.tail->next) {
        if (curr->special) {
            if (!strcmp(curr->token, ">")) {
                output_to_file(curr->next->token, false);
            }
            if (!strcmp(curr->token, ">>")) {
                output_to_file(curr->next->token, true);
            }
            if (!strcmp(curr->token, "<")) {
                file_to_input(curr->next->token);
            }
        }
        curr = curr->next;
    }

    if (bin_exists(cmd[0])) {
        execvp(cmd[0], cmd);
    } else {
        exit(0);
    }
    perror("could not exec in file_to_input\n");
    exit(-1);
}

/** Gets the next cmd subsection of the linked list based on whether
 *  it reaches the end or it finds a | */
static struct subsection get_next_subsection (tok_node *curr)
{
    struct subsection cmd;
    cmd.head = curr;
    int count = 0;

    tok_node *prev = NULL;
    while (curr != NULL) {
        if (curr->special) {
            if (!strcmp(curr->token, "|")) {
                cmd.tail = prev;
                break;
            }
        }
        prev = curr;
        curr = curr->next;
        if (curr == NULL) {
            cmd.tail = prev;
        }
        count++;
    }
    cmd.count = count;

    return cmd;
}

void execute (tok_node *head)
{
    /* find length of linked list */
    int ct = 0, pipe_ct = 0;
    count_tl(head, &ct, &pipe_ct);

    /* allocate storage for the cmd(s) */
    struct subsection cmds[pipe_ct+1];

    /* split input into separate cmds if pipe(s) */
    tok_node *curr = head;
    for (int i = 0; i < pipe_ct+1; i++) {
        cmds[i] = get_next_subsection(curr);
        curr = cmds[i].tail;
        if(curr->next != NULL) { // move to next non pipe token
            curr = curr->next->next;
        }
    }

    /* create variables for pipes */
    int pipefd[pipe_ct][2];

    for (int i = 0; i < pipe_ct; i++) {
        if (pipe(pipefd[i]) < 0) {
            perror("pipe failed");
        }
    }

    /* for forks */
    int status;
    pid_t pid;

    for (int i = 0; i < pipe_ct+1; i++) {
        fflush(NULL);
        pid = fork();
        if (pid < 0) {
            perror("fork() failed in execute\n");
            exit(-1);
        } else if (pid == 0) { // child
            if (i > 0) {
                if (dup2(pipefd[i-1][0], STDIN_FILENO) < 0) {
                    perror("dup2 failed");
                }
                close(pipefd[i-1][0]);
                close(pipefd[i-1][1]);
            }
            if (i+1 < pipe_ct+1) {
                close(pipefd[i][0]);
                if (dup2(pipefd[i][1], STDOUT_FILENO) < 0) {
                    perror("bottom dup2 failed");
                }
                close(pipefd[i][1]);
            }
            parse_cmd(cmds[i]);
            printf("exec failed\n");
        } else { // parent
            if (i > 0) {
                close(pipefd[i-1][0]);
                close(pipefd[i-1][1]);
            }
            while (1) {
                pid_t pidp = waitpid(pid, &status, 0);
                if (pidp <= 0) break;
            }
        }
    }

    return;
}

int main (int argc, char **argv)
{
    char userin[BUFF_SIZE];
    printf("Input something to be tokenized and run\n");
    fgets(userin, BUFF_SIZE, stdin);

    /* get tokenized input */
    tok_node *head = tokenize(userin);

    /* print the tokenized input */
//    print_tokens(head);

    /* exec tokenized input */
    execute(head);

    /* free the tokenized input */
    free_tokens(head);

    return 0;
}

/** count total tokens and pipes in a given tok_node
 * linked list */
static void count_tl (tok_node *head, int *ct, int *pipe_ct)
{
    while (head != NULL) {
        if (head->special) {
            if (!strcmp(head->token, "|")) {
                (*pipe_ct)++;
            }
        }
        head = head->next;
        (*ct)++;
    }
}

/* saves a path into a path_node linked list */
static void save_path(char *path, path_node **head)
{
    path_node *list = *head;
    path_node *p_node = malloc(sizeof(path_node)); // make new node
    int length = strlen(path);
    p_node->path = malloc(sizeof(char)*length+1); // make space for path
    strncpy(p_node->path, path, length+1); // put path in node
    p_node->path[length] = '\0'; // in case strncpy doesn't null terminate
    p_node->next = NULL; // set next to NULL, node is going at the end

    if (*head == NULL) { // if head is NULL, new node is head
        *head = p_node;
        return;
    } else { // otherwise insert node at the end
        while (list->next != NULL) {
            list = list->next;
        }
        list->next = p_node;
    }
    return;
}

/* gets the user's path environment variable and returns it
 * as a linked list */
static path_node* get_path()
{
    char *fpath = getenv("PATH"); // get path
    int length = strlen(fpath);
    char path[length]; // make buffer to store individual path strings
    path_node *head = NULL;

    /* go through full path(fpath) searching for colons, which are the
     * delimiters of the path environment variable */
    for (int i = 0, j = 0; i < length; i++) {
        if (fpath[i] == ':') {
            path[j] = '\0';
            save_path(path, &head);
            j = 0;
        } else {
            path[j++] = fpath[i];
        }
    }

    return head;
}

/* takes a path linked list and frees it */
static void free_path (path_node *phead)
{
    path_node *list = phead;
    while (list != NULL) {
        phead = phead->next;
        free(list->path);
        free(list);
        list = phead;
    }
}

/* checks the path environment variable to see if bin is in it */
static bool bin_exists (char *bin)
{
    /* get path in form of linked list */
    path_node *phead = get_path();

    bool found = false;
    DIR *dir;
    struct dirent *entry;
    path_node *list = phead;

    while (list != NULL && found == false) {
        if ((dir = opendir(list->path)) != NULL) {
            while ((entry = readdir(dir)) != NULL && found == false) {
                if (!strcmp(entry->d_name, bin)) {
                    found = true;
                }
            }
        }
        list = list->next;
    }
    closedir(dir);
    free_path(phead);
    return found;
}
