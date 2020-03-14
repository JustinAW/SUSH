/************************************************
 *       Shippensburg University Shell          *
 *                 executor.c                   *
 ************************************************
 * executor takes a linked list and runs the    *
 * strings stored in it as a command            *
 ************************************************
 * Author: Justin Weigle                        *
 * Edited: 13 Mar 2020                          *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../includes/sush.h"
#include "../includes/tokenizer.h"

void execute (tok_node *head)
{
    /* find length of linked list and
     * number of special tokens */
    int spec_ct = 0, ct = 0;
    tok_node *list = head;
    while (list != NULL) {
        if (list->special) {
            spec_ct++;
        }
        list = list->next;
        ct++;
    }
    /* allocate storage for the cmd(s)
     * and the special token location(s) */
    int spec_locs[spec_ct];
    char *cmd[ct];

    /* save cmd(s) and location(s) of
     * special token(s) */
    list = head;
    ct = 0, spec_ct = 0;
    while (list != NULL) {
        if (list->special) {
            spec_locs[spec_ct] = ct;
            spec_ct++;
        }
        cmd[ct] = list->token;
        list = list->next;
        ct++;
    }

    printf("special token locations:\n");
    for (int i = 0; i < spec_ct; i++) {
        printf("%d ", spec_locs[i]);
    }
    printf("\n");

    printf("all tokens:\n");
    for (int i = 0; i < ct; i++) {
        printf("%s ", cmd[i]);
    }
    printf("\n");

    for (int i = 0; i < spec_ct; i++) {
        if (!strcmp(cmd[spec_locs[i]], ">")) {
            printf("found >\n");
        }
        if (!strcmp(cmd[spec_locs[i]], ">>")) {
            printf("found >>\n");
        }
        if (!strcmp(cmd[spec_locs[i]], "<")) {
            printf("found <\n");
        }
        if (!strcmp(cmd[spec_locs[i]], "|")) {
            printf("found |\n");
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
