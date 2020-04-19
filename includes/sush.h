#ifndef SUSH_H
#define SUSH_H

#ifndef BUFF_SIZE
#define BUFF_SIZE 1025
#endif

#include <sys/resource.h>

enum RMANAGE {
    UPDATE,
    PRINT
};

void manage_rusage (enum RMANAGE, struct rusage);

void show_all_resources ();

#endif
