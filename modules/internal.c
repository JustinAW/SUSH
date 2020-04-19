/************************************************
 *       Shippensburg University Shell          *
 *                 internal.c                   *
 ************************************************
 * Handles internal commands for SUSH           *
 ************************************************
 * Author: Justin Weigle                        *
 *         Richard Bucco                        *
 * Edited: 19 Apr 2020                          *
 ************************************************/

#include "../includes/internal.h"
#include "../includes/sush.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static bool del_env_var (struct tok_list *);
static bool set_env_var (struct tok_list *);
static bool change_directory (struct tok_list *);
static bool print_wdirectory ();

/**
 * Runs a given internal command as long as it's
 * valid
 */
int run_internal_cmd (struct tok_list *tlist) {
    bool found_internal_cmd = false;
    bool err_found = false;
    if (!strcmp(tlist->head->token, "setenv")) {
        /* set an environment variable */
        err_found = set_env_var(tlist);
        found_internal_cmd = true;
    } else if (!strcmp(tlist->head->token, "unsetenv")) {
        /* delete an environment variable */
        err_found = del_env_var(tlist);
        found_internal_cmd = true;
    } else if (!strcmp(tlist->head->token, "cd")) {
        /* change directory */
        err_found = change_directory(tlist);
        found_internal_cmd = true;
    } else if (!strcmp(tlist->head->token, "pwd")) {
        /* print the current working directory */
        print_wdirectory();
        found_internal_cmd = true;
    } else if (!strcmp(tlist->head->token, "exit")) {
        /* print accounting info and exit */
        free_tok_list(tlist);
        found_internal_cmd = true;
        show_all_resources();
        exit(0);
    } else if (!strcmp(tlist->head->token, "accnt")) {
        /* print accounting info */
        show_all_resources();
        found_internal_cmd = true;
    }

    if (found_internal_cmd) {
        if (err_found) {
            return -1; // found, but error
        } else {
            return 0; // found, no error
        }
    } else {
        return 1; // not found
    }
}

/**
 * Add a new environment variable or modify an existing one
 */
static bool set_env_var (struct tok_list *tlist)
{
    if (tlist->count == 3) {
        if(setenv(tlist->head->next->token, tlist->tail->token, 1)) {
            perror("couldn't set env var");
            return true; // error
        }
    } else {
        fprintf(stderr, "setenv takes 2 arguments");
        return true; // error
    }
    return false; // no error
}

/**
 * Delete an environment variable
 */
static bool del_env_var (struct tok_list *tlist)
{
    if (tlist->count == 2) {
        if(unsetenv(tlist->tail->token)) {
            perror("couldn't delete");
            return true; // error
        }
    } else {
        fprintf(stderr, "unsetenv takes 1 argument");
        return true; // error
    }
    return false; // no error
}

/**
 * change to a give directory.
 * ~ == HOME
 */
static bool change_directory (struct tok_list *tlist)
{
    if (tlist->count == 2) {
        if ((tlist->head->next->token)[0] == '~') {
            const char *home = getenv("HOME");
            char tmpstr[strlen(home)+1];
            strcpy(tmpstr, home);
            const char *fpath = strcat(tmpstr, &(tlist->tail->token)[1]);
            chdir(fpath);
            return false; // no error
        }
        chdir(tlist->tail->token);
    } else {
        fprintf(stderr, "cd takes 1 argument\n");
        return true; // error
    }
    return false; // no error
}

/**
 * print the current working directory to STDOUT
 */
static bool print_wdirectory ()
{
    char buff[BUFF_SIZE];
    if (getcwd(buff, BUFF_SIZE) == NULL) {
        perror("couldn't get current working directory");
        return true; // error
    }
    printf("%s\n", buff);
    return false; // no error
}
