#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

typedef struct tok_node {
    char *token;
    bool special;
    struct tok_node *next;
} tok_node;

struct tok_list {
    tok_node *head;
    tok_node *tail;
    int count;
    int pcount;
};

void free_tok_list (struct tok_list*);

void tokenize (struct tok_list*, char*);

void print_tokens (tok_node*);

#endif
