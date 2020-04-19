/************************************************
 *       Shippensburg University Shell          *
 *                 rcreader.c                   *
 ************************************************
 * rcreader opens a .sushrc file in the user's  *
 * home directory as long as it is executable,  *
 * and then it calls tokenizer to split it into *
 * tokens to be passed to the executor          *
 ************************************************
 * Author: Justin Weigle                        *
 *         Richard Bucco                        *
 * Edited: 19 Apr 2020                          *
 ************************************************/

#include "../includes/rcreader.h"
#include "../includes/sush.h"
#include "../includes/executor.h"
#include "../includes/internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/resource.h>

/**
 * Opens the user's home directory using the $HOME environment
 *  variable and tries to find a .sushrc file that is executable
 *  to parse it into shell commands
 */
void read_sushrc ()
{
    struct tok_list tlist;
    tlist.head = NULL;
    tlist.tail = NULL;
    tlist.count = 0;

    /* set path to home and .sushrc */
    const char *home = getenv("HOME");
    // strcat cuts off \0 bit from *dest, need a temp
    int pathlen = strlen(home);
    char tmpstr[pathlen+1];
    strcpy(tmpstr, home);
    // store full path to .sushrc
    const char *rcfile = strcat(tmpstr, "/.sushrc");

    /* Search for .sushrc in $HOME */
    DIR *dir;
    struct dirent *entry;
    int found = 0;
    if ((dir = opendir(home)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (!strcmp(entry->d_name, ".sushrc")) {
                found = 1;
                // open if executable
                if (access(rcfile, X_OK) == 0) {
                    char buf[BUFF_SIZE];
                    FILE *fp = fopen(rcfile, "r");
                    // read file until EOF is found (fgets() returns NULL)
                    while ((fgets(buf, BUFF_SIZE, fp)) != NULL) {
                        tokenize(&tlist, buf);
                        if (tlist.head) {
                            int ret = run_internal_cmd(&tlist);
                            if (ret < 0) {
                                fprintf(stderr, "unable to run internal command\n");
                            } else if (ret == 0) {
                            } else {
                                execute(tlist.head);
                            }
                        }
                        free_tok_list(&tlist);
                    }
                    fclose(fp); // close the file
                } else {
                    perror("In read_sushrc() - .sushrc is not executable ");
                }
            }
        }
        if (!found) {
            fprintf(stderr, "No .sushrc found...\n");
        }
        closedir(dir);
    } else {
        perror("In read_sushrc() - Could not open $HOME ");
    }
}
