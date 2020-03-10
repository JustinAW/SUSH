#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct tok_node {
    char *token;
    int special;
    struct tok_node *next;
} tok_node;

typedef enum {
    Init_State,
    Blank_State,
    Letter_State,
    Redirect_State,
    Single_Quote_State,
    Double_Quote_State,
    Exit_State
} Token_Sys_State;

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
 * Uses state machine to tokenize a user's input into appropriate
 * tokens for processing as shell commands
 */
void tokenize (char *input)
{
    int length = strlen(input);
    if (input[length-1] != '\n') {
        fprintf(stderr, "Input didn't end in \\n, skipping...\n");
        return;
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
                printf("in init\n");
                if (ch == '"') {
                    State = Double_Quote_State;
                    start = i+1;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                    start = i+1;
                } else if (ch == '<' || ch == '>' || ch == '|') {
                    fprintf(stderr, "Need input before redirect or pipe\n");
                    return;
                } else if (ch == ' ') {
                } else if (32 <= ch <= 127) { //ascii vals
                    State = Letter_State;
                    start = i;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    return;
                }
                break;
            /** if redirect found : save string & redirect
             *  if space          : save string & blank
             *  if ascii          : nothing
             *  if next == \n     : save string & exit     */
            case Letter_State:
                printf("in letter\n");
                if (ch == '"') {
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
                } else if (32 <= ch <= 127) {
                    if (input[i+1] == '\n') {
                        State = Exit_State;
                        end = i+1;
                        save_string(start, end, input, &head, 0);
                    }
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    return;
                }
                break;
            /** if redirect found : redirect
             *  if space          : nothing
             *  if ascii          : letter     */
            case Blank_State:
                printf("in blank\n");
                if (ch == '"') {
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
                } else if (32 <= ch <= 127) {
                    State = Letter_State;
                    start = i;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    return;
                }
                break;
            /** if redirect found : error if prev not >
             *  if space          : eat spaces, nom nom
             *  if ascii          : save string & letter     */
            case Redirect_State:
                printf("in redirect\n");
                if (ch == '"') {
                    State = Double_Quote_State;
                    save_string(start, end, input, &head, 1);
                    start = i+1;
                } else if (ch == '\'') {
                    State = Single_Quote_State;
                    save_string(start, end, input, &head, 1);
                    start = i+1;
                } else if (input[i] == '\n') {
                    fprintf(stderr, "Can't have redirect at end\n");
                    return;
                } else if (ch == '<' || ch == '|') {
                    fprintf(stderr, "%c not valid after %c\n", ch, input[i-1]);
                    return;
                } else if (ch == '>') {
                    if (input[i-1] == '>') {
                        if (input[i+1] == '>') {
                            fprintf(stderr, "Too many redirects in a row\n");
                            return;
                        }
                        end = i+1;
                    } else {
                        fprintf(stderr,
                            "Cannot have spaces between >\n", ch);
                        return;
                    }
                } else if (ch == ' ') {
                } else if (32 <= ch <= 127) {
                    State = Letter_State;
                    save_string(start, end, input, &head, 1);
                    start = i;
                } else {
                    fprintf(stderr, "Unrecognized character %c\n", ch);
                    return;
                }
                break;
            /** everything is treated as plaintext until
             *  the next single quote, then save string.
             *  After end quote found, check next char to
             *  decide which state to change to             */
            case Single_Quote_State:
                printf("in single quote\n");
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
                    } else if (32 <= ch <= 127) {
                        State = Letter_State;
                    } else {
                        fprintf(stderr, "Unrecognized character %c\n", ch);
                        return;
                    }
                } else if (ch == '\n') {
                    fprintf(stderr, "Quote never closed \'\n");
                    return;
                }
                break;
            /** see single quote explanation    */
            case Double_Quote_State:
                printf("in double quote\n");
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
                    } else if (32 <= ch <= 127) {
                        State = Letter_State;
                    } else {
                        fprintf(stderr, "Unrecognized character %c\n", ch);
                        return;
                    }
                } else if (ch == '\n') {
                    fprintf(stderr, "Quote never closed \"\n");
                    return;
                }
                break;
            case Exit_State:
                printf("in exit\n");
                break;
            default:
                break;
        }
    }
    tok_node *list = head;
    while (list != NULL) {
        printf("%s \t: %d\n", list->token, list->special);
        list = list->next;
    }
}

int main (int argc, char **argv)
{
    char userin[1025];

    fgets(userin, 1025, stdin);

    tokenize(userin);

    return EXIT_SUCCESS;
}
