/************************************************
 *       Shippensburg University Shell          *
 *                tokenizer.c                   *
 ************************************************
 * tokenize takes a string as input and then    *
 * separates the string into tokens that can    *
 * further be used to execute shell commands    *
 ************************************************
 * Author: Justin Weigle                        *
 * Edited: 11 Mar 2020                          *
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

void save_string (int, int, char*, tok_node**, int);
void error_free (tok_node**);

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

    tok_node *head = NULL;
    char ch;
    int start, end, i;
    Token_Sys_State State = Init_State;
    for(i = 0; i < length; i++) {
        ch = input[i];
        switch (State) {
            /** if redirect found : error
             *  if space          : nothing
             *  if ascii          : letter     */
            case Init_State:
                if (ch == '\n') {
                    return NULL;
                } else if (ch == '"') {
                    State = Double_Quote_State;
                    start = i+1;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                    start = i+1;
                } else if (ch == '<' || ch == '>' || ch == '|') {
                    fprintf(stderr, "Need input before redirect or pipe\n");
                    error_free(&head);;
                    return NULL;
                } else if (ch == ' ') {
                } else if (32 <= ch && ch <= 127) { //ascii vals
                    State = Letter_State;
                    start = i;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    error_free(&head);;
                    return NULL;
                }
                break;
            /** if redirect found : save string & redirect
             *  if space          : save string & blank
             *  if ascii          : nothing
             *  if next == \n     : save string & exit     */
            case Letter_State:
                if (ch == '\n') {
                    end = i;
                    save_string(start, end, input, &head, 0);
                } else if (ch == '"') {
                    State = Double_Quote_State;
                    save_string(start, end, input, &head, 0);
                    start = i+1;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                    save_string(start, end, input, &head, 0);
                    start = i+1;
                } else if (ch == '<' || ch == '>' || ch == '|') {
                    State = Redirect_State;
                    end = i;
                    save_string(start, end, input, &head, 0);
                    start = i;
                    end = i+1;
                } else if (ch == ' ') {
                    State = Blank_State;
                    end = i;
                    save_string(start, end, input, &head, 0);
                } else if (32 <= ch && ch <= 127) {
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    error_free(&head);;
                    return NULL;
                }
                break;
            /** if redirect found : redirect
             *  if space          : nothing
             *  if ascii          : letter     */
            case Blank_State:
                if (ch == '\n') {
                    end = i;
                    save_string(start, end, input, &head, 0);
                } else if (ch == '"') {
                    State = Double_Quote_State;
                    start = i+1;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                    start = i+1;
                } else if (ch == '<' || ch == '>' || ch == '|') {
                    State = Redirect_State;
                    start = i;
                    end = i+1;
                } else if (ch == ' ') {
                } else if (32 <= ch && ch <= 127) {
                    State = Letter_State;
                    start = i;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    error_free(&head);;
                    return NULL;
                }
                break;
            /** if redirect found : error if prev not >
             *  if space          : eat spaces, nom nom
             *  if ascii          : save string & letter     */
            case Redirect_State:
                if (ch == '"') {
                    State = Double_Quote_State;
                    save_string(start, end, input, &head, 1);
                    start = i+1;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                    save_string(start, end, input, &head, 1);
                    start = i+1;
                } else if (ch == '\n') {
                    fprintf(stderr, "Can't have redirect at end\n");
                    error_free(&head);;
                    return NULL;
                } else if (ch == '<' || ch == '|') {
                    fprintf(stderr, "%c not valid after %c\n", ch, input[i-1]);
                    error_free(&head);;
                    return NULL;
                } else if (ch == '>') {
                    if (input[i-1] == '>') {
                        if (input[i+1] == '>') {
                            fprintf(stderr, "Too many redirects in a row\n");
                            error_free(&head);;
                            return NULL;
                        }
                        end = i+1;
                    } else {
                        fprintf(stderr,
                            "Cannot have spaces between >\n");
                        error_free(&head);;
                        return NULL;
                    }
                } else if (ch == ' ') {
                } else if (32 <= ch && ch <= 127) {
                    State = Letter_State;
                    save_string(start, end, input, &head, 1);
                    start = i;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    error_free(&head);;
                    return NULL;
                }
                break;
            /** everything is treated as plaintext until
             *  the next single quote, then save string.
             *  After end quote found, check next char to
             *  decide which state to change to             */
            case Single_Quote_State:
                if (ch == '\'') {
                    end = i;
                    save_string(start, end, input, &head, 0);
                    i++; // only way to handle possible two quotes in a row
                    start = i;
                    ch = input[i];
                    if (ch == '"') {
                        start = i+1;
                        State = Double_Quote_State;
                    } else if (ch == '\'') {
                        start = i+1; //and stay in single quote state
                    } else if (ch == '<' || ch == '>' || ch == '|') {
                        State = Redirect_State;
                        end = i+1;
                    } else if (ch == ' ') {
                        State = Blank_State;
                    } else if (32 <= ch && ch <= 127) {
                        State = Letter_State;
                    } else if (ch == '\n') {
                    } else {
                        fprintf(stderr, "Unrecognized character %c\n", ch);
                        error_free(&head);;
                        return NULL;
                    }
                } else if (ch == '\n') {
                    fprintf(stderr, "Quote never closed \'\n");
                    error_free(&head);;
                    return NULL;
                }
                break;
            /** see single quote explanation    */
            case Double_Quote_State:
                if (ch == '"') {
                    end = i;
                    save_string(start, end, input, &head, 0);
                    i++; // only way to handle possible two quotes in a row
                    start = i;
                    ch = input[i];
                    if (ch == '\'') {
                        start = i+1;
                        State = Single_Quote_State;
                    } else if (ch == '"') {
                        start = i+1; //and stay in double quote state
                    } else if (ch == '<' || ch == '>' || ch == '|') {
                        State = Redirect_State;
                        end = i+1;
                    } else if (ch == ' ') {
                        State = Blank_State;
                    } else if (32 <= ch && ch <= 127) {
                        State = Letter_State;
                    } else if (ch == '\n') {
                    } else {
                        fprintf(stderr, "Unrecognized character %c\n", ch);
                        error_free(&head);;
                        return NULL;
                    }
                } else if (ch == '\n') {
                    fprintf(stderr, "Quote never closed \"\n");
                    error_free(&head);;
                    return NULL;
                }
                break;
            default:
                break;
        }
    }
    return head;
}

/**
 * Inserts a new node at the end of the linked list pointed to
 * by head with the string in input between start(inclusive) and
 * end(exclusive)
 */
void save_string (int start, int end, char *input, tok_node **head, int spec)
{
    tok_node *t_node = malloc(sizeof(tok_node)); // make new node
    int length = end - start;
    t_node->token = malloc(sizeof(char)*length+1); // make space for token
    strncpy(t_node->token, &input[start], length+1); // put token in node
    t_node->token[length] = '\0'; // strncpy doesn't null terminate
    t_node->special = spec; // set if token is special
    t_node->next = NULL; // set next to NULL, node is going at the end
    if (*head == NULL) { // if head is NULL, new node is head
        *head = t_node;
        return;
    } else { // otherwise insert node at the end
        tok_node *list = *head;
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
void error_free (tok_node **head)
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
