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
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../includes/sush.h"
#include "../includes/tokenizer.h"

/** count total tokens and special tokens in a given tok_node
 * linked list */
void count_ll (tok_node *head, int *ct, int *spec_ct)
{
    while (head != NULL) {
        if (head->special) {
            (*spec_ct)++;
        }
        head = head->next;
        (*ct)++;
    }
}

/** redirect stdout to the file listed after >
 *  append controls whether the file is appended or overwritten */
void output_to_file (tok_node *list, char **cmd, int ct, bool append)
{
    int fd;
    if (append) {
        fd = open(list->token, O_WRONLY | O_CREAT | O_APPEND,
                S_IWUSR | S_IRUSR);
    } else {
        fd = open(list->token, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
    }
    if (fd < 0) {
        perror("open failed in execute > output_to_file\n");
        exit(-1);
    }

    dup2(fd, STDOUT_FILENO); // stdout > file
    close(fd); // done, connection made with dup2
    cmd[ct] = NULL;
    execvp(cmd[0], cmd);
    perror("could not exec in output_to_file\n");
    exit(-1);
}

/** redirect a file to stdin */
void file_to_input (tok_node *list, char **cmd, int ct)
{
    int fd;
    fd = open(list->token, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        perror("open failed in execute > file_to_input\n");
        exit(-1);
    }

    dup2(fd, STDIN_FILENO); // stdin < file
    close(fd); // done, connection made with dup2
    cmd[ct] = NULL;
    execvp(cmd[0], cmd);
    perror("could not exec in file_to_input\n");
    exit(-1);
}

void execute (tok_node *head)
{
    /* find length of linked list */
    int ct = 0, spec_ct = 0;
    count_ll(head, &ct, &spec_ct);

    /* allocate storage for the cmd(s)
     * and the special token location(s) */
    char *cmd[ct+1];
    char *spec[1];

    /* create variables for possible pipes
     * and redirects */
    int pipefd[2];
    /* for forks */
    int status;
    pid_t pid;

    /* fork() and exec() + handle redirs & pipes */
    tok_node *list = head;
    ct = 0;
    while (list != NULL) {
        if (list->special) {
            *spec = list->token;
            list = list->next;
            pid = fork();
            if (pid < 0) {
                perror("fork() failed in execute\n");
                exit(-1);
            } else if (pid == 0) { // child
                if (!strcmp(*spec, ">")) {
                    output_to_file(list, cmd, ct, false);
                }
                if (!strcmp(*spec, ">>")) {
                    output_to_file(list, cmd, ct, true);
                }
                if (!strcmp(*spec, "<")) {
                    file_to_input(list, cmd, ct);
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
            ct = 0;
            list = list->next;
        } else if (list->next == NULL && spec_ct == 0) {
            cmd[ct++] = list->token;
            cmd[ct] = NULL;
            pid = fork();
            if (pid < 0) {
                perror("fork() failed in execute\n");
                exit(-1);
            } else if (pid == 0) { // child
                execvp(cmd[0], cmd);
            } else { // parent
                while (1) {
                    pid_t pidp = waitpid(pid, &status, 0);
                    if (pidp <= 0) break;
                }
            }
            list = list->next;
        } else {
            cmd[ct++] = list->token;
            list = list->next;
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
    tok_node *head;
    head = tokenize(userin);

    /* print the tokenized input */
    tok_node *list = head;
    while (list != NULL) {
        printf("%d:%s\n", list->special, list->token);
        list = list->next;
    }

    execute(head);

    /* free the nodes */
    list = head;
    while (list != NULL) {
        head = head->next;
        free(list);
        list = head;
    }

    return 0;
}
