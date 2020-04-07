#ifndef INTERNAL_H
#define INTERNAL_H

typedef struct envvar_node {
    char *name;
    char *value;
    struct envvar_node *next;
} envvar_node;

struct env_var {
    char *name;
    char *value;
};

envvar_node* setenv_var (envvar_node *, struct env_var);

envvar_node* unsetenv_var (envvar_node *, char *);

#endif
