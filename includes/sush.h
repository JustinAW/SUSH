#ifndef SUSH_H
#define SUSH_H

#ifndef BUFF_SIZE
#define BUFF_SIZE 1025
#endif

enum RMANAGE {
    UPDATE,
    PRINT
};

void manage_rusage (enum RMANAGE, struct rusage);

#endif
