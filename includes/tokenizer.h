#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef struct tok_node {
    char *token;
    int special;
    struct tok_node *next;
} tok_node;

tok_node* tokenize (char*);

#endif