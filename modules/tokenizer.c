/************************************************
 *       Shippensburg University Shell          *
 *                tokenizer.c                   *
 ************************************************
 * tokenize takes a string as input and then    *
 * separates the string into tokens that can    *
 * further be used to execute shell commands    *
 ************************************************
 * Author: Justin Weigle                        *
 * Edited: 14 Mar 2020                          *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../includes/sush.h"
#include "../includes/tokenizer.h"

typedef enum {
    Init_State,
    Blank_State,
    Letter_State,
    Redirect_State,
    Single_Quote_State,
    Double_Quote_State,
} Token_Sys_State;

static void save_string (char*, tok_node**, int);
static void error_free_ll (tok_node**);

/**
 * Uses state machine to tokenize a user's input into appropriate
 * tokens for processing as shell commands
 */
tok_node* tokenize (char *input)
{
    int length = strlen(input);
    if (input[length-1] != '\n') {
        fprintf(stderr, "Input didn't end in \\n, skipping...\n");
        return NULL;
    }

    char token[length];
    tok_node *head = NULL;
    char ch;
    Token_Sys_State State = Init_State;

    for(int i = 0, j = 0; i < length; i++) {
        ch = input[i];
        switch (State) {
            /* Should only change for quotes or letters.
             * Save ch if letter */
            case Init_State:
                if (ch == '\n') {
                    return NULL;
                } else if (ch == '"') {
                    State = Double_Quote_State;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                } else if (ch == '<' || ch == '>' || ch == '|') {
                    fprintf(stderr, "Need input before redirect or pipe\n");
                    return NULL;
                } else if (ch == ' ') {
                } else if (32 <= ch && ch <= 127) {
                    State = Letter_State;
                    token[j] = ch;
                    j++;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    return NULL;
                }
                break;
            /* Change for everything except letters.
             * Save ch for letters only.
             * Save string for redirect and newline */
            case Letter_State:
                if (ch == '\n') {
                    token[j] = '\0';
                    save_string(token, &head, 0);
                } else if (ch == '"') {
                    State = Double_Quote_State;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                } else if (ch == '<' || ch == '>' || ch == '|') {
                    State = Redirect_State;
                    token[j] = '\0';
                    save_string(token, &head, 0);
                    token[0] = ch;
                    j = 1;
                } else if (ch == ' ') {
                    State = Blank_State;
                    token[j] = '\0';
                    save_string(token, &head, 0);
                    j = 0;
                } else if (32 <= ch && ch <= 127) {
                    token[j] = ch;
                    j++;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    error_free_ll(&head);;
                    return NULL;
                }
                break;
            /* Change for everything except blanks.
             * Save ch for letters and redirect */
            case Blank_State:
                if (ch == '\n') {
                } else if (ch == '"') {
                    State = Double_Quote_State;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                } else if (ch == '<' || ch == '>' || ch == '|') {
                    State = Redirect_State;
                    token[j] = ch;
                    j++;
                } else if (ch == ' ') {
                } else if (32 <= ch && ch <= 127) {
                    State = Letter_State;
                    token[j] = ch;
                    j++;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    error_free_ll(&head);
                    return NULL;
                }
                break;
            /* save ch for another > if >.
             * error on any other redirect or newline
             * save string and change for anything else
             * */
            case Redirect_State:
                if (ch == '"') {
                    State = Double_Quote_State;
                    token[j] = '\0';
                    save_string(token, &head, 1);
                    j = 0;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                    token[j] = '\0';
                    save_string(token, &head, 1);
                    j = 0;
                } else if (ch == '\n') {
                    fprintf(stderr, "Can't have redirect at end of input\n");
                    error_free_ll(&head);;
                    return NULL;
                } else if (ch == '<' || ch == '|') {
                    fprintf(stderr, "%c not valid after >\n", ch);
                    error_free_ll(&head);;
                    return NULL;
                } else if (ch == '>') {
                    if (input[i-1] == '>') {
                        if (input[i+1] == '>') {
                            fprintf(stderr, "Too many redirects in a row\n");
                            error_free_ll(&head);;
                            return NULL;
                        }
                        token[j] = ch;
                        j++;
                    } else {
                        fprintf(stderr,
                            "Cannot have spaces between >\n");
                        error_free_ll(&head);;
                        return NULL;
                    }
                } else if (ch == ' ') {
                } else if (32 <= ch && ch <= 127) {
                    State = Letter_State;
                    token[j] = '\0';
                    save_string(token, &head, 1);
                    token[0] = ch;
                    j = 1;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    error_free_ll(&head);;
                    return NULL;
                }
                break;
            /** everything is treated as plaintext until
             *  the next single quote other than escaped chars.
             *  After end quote found, check next char to
             *  decide which state to change to             */
            case Single_Quote_State:
                if (ch == '\'') {
                    i++; // check next character
                    ch = input[i];
                    if (ch == '"') {
                        State = Double_Quote_State;
                    } else if (ch == '\'') {
                    } else if (ch == '<' || ch == '>' || ch == '|') {
                        State = Redirect_State;
                        token[j] = '\0';
                        save_string(token, &head, 0);
                        token[0] = ch;
                        j = 1;
                    } else if (ch == ' ') {
                        State = Blank_State;
                        token[j] = '\0';
                        save_string(token, &head, 0);
                        j = 0;
                    } else if (32 <= ch && ch <= 127) {
                        State = Letter_State;
                        token[j] = ch;
                        j++;
                    } else if (ch == '\n') {
                        token[j] = '\0';
                        save_string(token, &head, 0);
                    } else {
                        fprintf(stderr, "Unrecognized character %c\n", ch);
                        error_free_ll(&head);;
                        return NULL;
                    }
                } else if (ch == '\n') {
                    fprintf(stderr, "Quote never closed \'\n");
                    error_free_ll(&head);;
                    return NULL;
                } else if (ch == '\\') {
                    i++;
                    token[j] = input[i];
                    j++;
                } else {
                    token[j] = ch;
                    j++;
                }
                break;
            /** see single quote explanation    */
            case Double_Quote_State:
                if (ch == '"') {
                    i++; // check next character
                    ch = input[i];
                    if (ch == '\'') {
                        State = Single_Quote_State;
                    } else if (ch == '"') {
                    } else if (ch == '<' || ch == '>' || ch == '|') {
                        State = Redirect_State;
                        token[j] = '\0';
                        save_string(token, &head, 0);
                        token[0] = ch;
                        j = 1;
                    } else if (ch == ' ') {
                        State = Blank_State;
                        token[j] = '\0';
                        save_string(token, &head, 0);
                        j = 0;
                    } else if (32 <= ch && ch <= 127) {
                        State = Letter_State;
                        token[j] = ch;
                        j++;
                    } else if (ch == '\n') {
                        token[j] = '\0';
                        save_string(token, &head, 0);
                    } else {
                        fprintf(stderr, "Unrecognized character %c\n", ch);
                        error_free_ll(&head);;
                        return NULL;
                    }
                } else if (ch == '\n') {
                    fprintf(stderr, "Quote never closed \"\n");
                    error_free_ll(&head);;
                    return NULL;
                } else if (ch == '\\') {
                    i++;
                    token[j] = input[i];
                    j++;
                } else {
                    token[j] = ch;
                    j++;
                }
                break;
            default:
                break;
        }
    }
    return head;
}

/**
 * Inserts a new node at the end of the linked list pointed to by head
 * with the given string token and marks whether it is a special
 * token or not based on spec */
static void save_string (char *token, tok_node **head, int spec)
{
    // TODO in this function: error checking
    tok_node *list = *head;
    tok_node *t_node = malloc(sizeof(tok_node)); // make new node
    int length = strlen(token);
    t_node->token = malloc(sizeof(char)*length+1); // make space for token
    strncpy(t_node->token, token, length+1); // put token in node
    t_node->token[length] = '\0'; // in case strncpy doesn't null terminate
    t_node->special = spec; // set if token is special
    t_node->next = NULL; // set next to NULL, node is going at the end

    if (*head == NULL) { // if head is NULL, new node is head
        *head = t_node;
        return;
    } else { // otherwise insert node at the end
        while (list->next != NULL) {
            list = list->next;
        }
        list->next = t_node;
    }
    return;
}

/**
 * called when an error occurs to free up memory
 */
static void error_free_ll (tok_node **head)
{
    tok_node *temp = *head;
    while (temp != NULL) {
        *head = (*head)->next;
        free(temp);
        temp = *head;
    }
    return;
}

/**
 * exemplifies how to use the tokenizer and then
 * free up the memory
 */
//int main (int argc, char **argv)
//{
//    /* get user input */
//    char userin[BUFF_SIZE];
//    printf("Input something to be tokenized\n");
//    fgets(userin, BUFF_SIZE, stdin);
//
//    /* get tokenized input */
//    tok_node *tok_input;
//    tok_input = tokenize(userin);
//
//    /* print the tokenized input */
//    tok_node *list = tok_input;
//    while (list != NULL) {
//        printf("%s \t: %d\n", list->token, list->special);
//        list = list->next;
//    }
//
//    /* free the nodes */
//    list = tok_input;
//    while (tok_input != NULL) {
//        list = tok_input->next;
//        free(tok_input);
//        tok_input = list;
//    }
//
//    return 0;
//}
