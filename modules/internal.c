#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../includes/internal.h"

static envvar_node* add_env_var (envvar_node *, struct env_var);

/**
 * set an environment variable
 */
envvar_node* setenv_var (envvar_node *head, struct env_var new_env_var)
{
    envvar_node *curr = head;
    while (curr != NULL) {
        if (!strcmp(curr->name, new_env_var.name)) { // if variable is found
            int length = strlen(new_env_var.value);
            free(curr->value); // delete current value
            curr->value = malloc(sizeof(char) * length + 1); // allocate for new value
            if (curr->value == NULL) {
                perror("malloc failed in env_var");
                exit(-1);
            }
            strncpy(curr->name, new_env_var.name, length+1); // put in new value
            curr->name[length] = '\0'; // make sure strncpy NULL terminated
            break;
        }
        curr = curr->next;
    }
    if (curr == NULL) { // if the end of the list was found, var not found
        head = add_env_var(head, new_env_var); // add a new one
    }

    return head;
}

/**
 * Delete an environment variable *name
 */
envvar_node* unsetenv_var (envvar_node *head, char *name)
{
    bool found = false;
    envvar_node *prev = NULL;
    envvar_node *curr = head;
    while (curr != NULL) {
        if (!strcmp(curr->name, name)) { // when name is found
            found = true;
            if (prev == NULL) {
                head = curr->next; // delete from beginning
            } else if (curr->next == NULL) {
                prev->next = NULL; // delete from end
            } else {
                prev->next = curr->next; // delete from middle
            }
            free(curr->name);
            free(curr->value);
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    if (!found) { // didn't find the environment variable
        fprintf(stderr, "env var %s not found\n", name);
    }
    return head;
}

/**
 * add a new environment variable
 */
static envvar_node* add_env_var (envvar_node *head, struct env_var new_env_var)
{
    envvar_node *new_node = malloc(sizeof(envvar_node));
    /* get lengths of the new variable */
    int name_len = strlen(new_env_var.name);
    int val_len = strlen(new_env_var.value);

    /* set new environment variable node's name */
    new_node->name = malloc(sizeof(char) * name_len + 1);
    if (new_node->name == NULL) {
        perror("malloc failed in env_var");
        exit(-1);
    }
    strncpy(new_node->name, new_env_var.name, name_len+1);
    new_node->name[name_len] = '\0';

    /* set new environment variable node's value */
    new_node->value = malloc(sizeof(char) * val_len + 1);
    if (new_node->value == NULL) {
        perror("malloc failed in env_var");
        exit(-1);
    }
    strncpy(new_node->value, new_env_var.value, val_len+1);
    new_node->value[val_len] = '\0';
    new_node->next = NULL;

    if (head == NULL) {
        head = new_node;
        return head;
    } else {
        new_node->next = head; // set new node's next to head
        head = new_node; // set head to new node
    }
    return head;
}

/*
void print_envnodes (envvar_node *head) {
    while(head != NULL) {
        printf("name: %s\n", head->name);
        printf("value: %s\n", head->value);
        head = head->next;
    }
}
int main (int argc, char **argv)
{
    envvar_node *head = NULL;
    struct env_var new_node;
    new_node.name = "Pickle";
    new_node.value = "Rick";
    head = setenv_var(head, new_node);

    struct env_var new_node2;
    new_node2.name = "Tiny";
    new_node2.value = "Rick";
    head = setenv_var(head, new_node2);

//    print_envnodes(head);

    head = unsetenv_var(head, "Pickle");
    head = unsetenv_var(head, "Tiny");

    return 0;
}
*/
