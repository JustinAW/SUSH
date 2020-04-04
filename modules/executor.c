/************************************************
 *       Shippensburg University Shell          *
 *                 executor.c                   *
 ************************************************
 * executor takes a linked list and runs the    *
 * strings stored in it as a command            *
 ************************************************
 * Author: Justin Weigle                        *
 * Edited: 16 Mar 2020                          *
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

static void count_tl (tok_node *, int *, int *);
static void output_to_file (tok_node *, char **, int, bool, path_node *);
static void file_to_input (tok_node *, char **, int, path_node *);
static void save_path(char *, path_node **);
static path_node* get_path();
static void free_path (path_node *);
static bool bin_exists (path_node *, char *);

enum Read_Write {
    READ,
    WRITE
};

struct subsection {
    tok_node *head;
    tok_node *tail;
};

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

    return 0;
}

/** redirect stdout to the file listed after >
 *  append controls whether the file is appended or overwritten */
static void output_to_file (tok_node *list, char **cmd, int ct, bool append, path_node *phead)
{
    int fd;
    list = list->next;
    fd = get_fd(list->token, WRITE, append);

    dup2(fd, STDOUT_FILENO); // stdout > file
    close(fd); // done, connection made with dup2
    cmd[ct] = NULL;

    if (bin_exists(phead, cmd[0])) {
        execvp(cmd[0], cmd);
    } else {
        exit(0);
    }
    perror("could not exec in output_to_file\n");
    exit(-1);
}

/** redirect a file to stdin */
static void file_to_input (tok_node *list, char **cmd, int ct, path_node *phead)
{
    int fd;
    list = list->next;
    fd = get_fd(list->token, READ, false);

    dup2(fd, STDIN_FILENO); // stdin < file
    close(fd); // done, connection made with dup2
    cmd[ct] = NULL;

    if (bin_exists(phead, cmd[0])) {
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

    tok_node *prev = curr;
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
    }

    return cmd;
}

void execute (tok_node *head)
{
    /* get path in form of linked list */
    path_node *phead = get_path();

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
        pid = fork();
        if (pid < 0) {
            perror("fork() failed in execute\n");
            exit(-1);
        } else if (pid == 0) { // child
            if (i > 0) {
                dup2(pipefd[i-1][0], STDIN_FILENO);
                close(pipefd[i-1][0]);
                close(pipefd[i-1][1]);
            }
            if (i+1 < pipe_ct+1) {
                close(pipefd[i][0]);
                dup2(pipefd[i][1], STDOUT_FILENO);
                close(pipefd[i][1]);
            }
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

    printf("made it past pipe section\n");
    free_path(phead);
    return;
}
/*
            pid = fork();
            if (pid < 0) {
                perror("fork() failed in execute\n");
                exit(-1);
            } else if (pid == 0) { // child
                if (!strcmp(*spec, ">")) {
                    output_to_file(list, cmd, ct, false, phead);
                }
                if (!strcmp(*spec, ">>")) {
                    output_to_file(list, cmd, ct, true, phead);
                }
                if (!strcmp(*spec, "<")) {
                    file_to_input(list, cmd, ct, phead);
                }
                if (!strcmp(*spec, "|")) {
                    printf("found |\n");
                }
                exit(0);
            } else { // parent
                while (1) {
                    pid_t pidp = waitpid(pid, &status, 0);
                    if (pidp <= 0) break;
                }
            }
        } else {
            cmd[ct++] = list->token;
            list = list->next;
        }
*/

int main (int argc, char **argv)
{
    char userin[BUFF_SIZE];
    printf("Input something to be tokenized and run\n");
    fgets(userin, BUFF_SIZE, stdin);

    /* get tokenized input */
    tok_node *head = tokenize(userin);

    /* print the tokenized input */
    print_tokens(head);

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
static bool bin_exists (path_node *phead, char *bin)
{
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
    return found;
}
