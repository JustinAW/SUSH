#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

int main (int argc, char **argv)
{
    // set path and some things up
    const char *home = getenv("HOME");
    // strcat cuts off dest \0 bit, need a temp
    int pathlen = strlen(home);
    char tmpstr[pathlen+1];
    strcpy(tmpstr, home);
    // store full path to .sushrc
    const char *rcfile = strcat(tmpstr, "/.sushrc");

    // Search for .sushrc in $HOME
    FILE *fp;
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(home)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (!strcmp(entry->d_name, ".sushrc")) {
                // open if executable
                if (access(rcfile, X_OK) == 0) {
                    fp = fopen(rcfile, "r");
                    printf("file is executable\n");
                    fclose(fp); // close the file
                } else {
                    printf("file not executable\n");
                }
            }
        }
        closedir(dir);
    } else {
        perror("Could not find .sushrc in $HOME/ ");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
