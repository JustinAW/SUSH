#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include "sush.h"
#include "tokenizer.h"
#include "rcreader.h"

void print_and_free (tok_node *head)
{
    tok_node *list = head;
    while (list != NULL) {
        printf("%s \t: %d\n", list->token,
               list->special);
        list = list->next;
    }
    list = head;
    int j = 1;
    while (list != NULL) {
        printf("deleting node %d\n", j);
        head = head->next;
        free(list);
        list = head;
        j++;
    }
}

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
                                // TODO call executor here
                                tok_node *head = tokenize(buf);
                                print_and_free(head); //instead of this
                            } else {
                                while (fgetc(fp) != '\n') { }
                                fprintf(stderr, "maxlen 1023, skipping...\n");
                            }
                        } else {
                            // TODO call executor here
                            tok_node *head = tokenize(buf);
                            print_and_free(head); //instead of this
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
