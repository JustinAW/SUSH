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
 * Edited: 14 Mar 2020                          *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include "../includes/sush.h"
#include "../includes/tokenizer.h"
#include "../includes/rcreader.h"

/**
 * Opens the user's home directory using the $HOME environment
 *  variable and tries to find a .sushrc file that is executable
 *  to parse it into shell commands
 */
void read_sushrc ()
{
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
    int length = 0;
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
                        length = strlen(buf);
                        if (length == 1024) {
                            if (buf[length-1] == '\n') {
                                tok_node *head = tokenize(buf);
                                // TODO call executor here
                                free_tokens(head);
                            } else {
                                while (fgetc(fp) != '\n') { }
                                fprintf(stderr, "maxlen 1023, skipping...\n");
                            }
                        } else {
                            tok_node *head = tokenize(buf);
                            // TODO call executor here
                            free_tokens(head);
                        }
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
