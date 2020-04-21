/************************************************
 *       Shippensburg University Shell          *
 *                   sush.c                     *
 ************************************************
 * Linux shell command interpreter.             *
 * Reads a .sushrc file from the user's home    *
 * directory and execs it line by line if it is *
 * executable, then waits for user input to     *
 * tokenize and run commands                    *
 ************************************************
 * Author: Justin Weigle                        *
 *         Richard Bucco                        *
 * Edited: 19 Apr 2020                          *
 ************************************************/

#include "includes/sush.h"
#include "includes/tokenizer.h"
#include "includes/executor.h"
#include "includes/internal.h"
#include "includes/rcreader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

static void print_resources (struct rusage);
static void show_resources (int);
static void show_child_resources (int);

int main (int argc, char **argv)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, show_resources);
    signal(SIGUSR2, show_all_resources);
    signal(SIGCHLD, show_child_resources);

    read_sushrc();

    struct tok_list tlist;
    tlist.head = NULL;
    tlist.tail = NULL;
    tlist.count = 0;
    tlist.pcount = 0;

    char userin[BUFF_SIZE];
    while (!feof(stdin)) {
        char *PS1 = getenv("PS1");
        if (PS1 == NULL) {
            printf("$ ");
        } else {
            printf("%s", PS1);
        }

        /* get user input */
        if (fgets(userin, BUFF_SIZE, stdin) == NULL) {
            exit(0);
        }

        /* tokenize the input */
        tokenize(&tlist, userin);

        /* print the tokenized input */
//        print_tokens(tlist.head);

        /* exec tokenized input */
        if (tlist.head) {
            int ret = run_internal_cmd(&tlist);
            if (ret < 0) {
                fprintf(stderr,"Unable to run internal command\n");
            } else if (ret > 0) { // wasn't an internal command
                execute(&tlist);
            }
        }

        /* free the tokenized input */
        free_tok_list(&tlist);
    }

    return 0;
}

/**
 * Uses static storage to track resource usage across the
 * entire runtime
 */
void manage_rusage (enum RMANAGE setting, struct rusage usage)
{
    static struct rusage total_usage;
    time_t time1;
    time_t time2;

    if (setting == UPDATE) {
        time1 = usage.ru_utime.tv_sec;
        time2 = usage.ru_utime.tv_usec;
        total_usage.ru_utime.tv_sec += time1;
        total_usage.ru_utime.tv_usec += time2;

        time1 = usage.ru_stime.tv_sec;
        time2 = usage.ru_stime.tv_usec;
        total_usage.ru_stime.tv_sec += time1;
        total_usage.ru_stime.tv_usec += time2;

        total_usage.ru_maxrss   +=  usage.ru_maxrss;
        total_usage.ru_ixrss    +=  usage.ru_ixrss;
        total_usage.ru_idrss    +=  usage.ru_idrss;
        total_usage.ru_isrss    +=  usage.ru_isrss;
        total_usage.ru_minflt   +=  usage.ru_minflt;
        total_usage.ru_majflt   +=  usage.ru_majflt;
        total_usage.ru_nswap    +=  usage.ru_nswap;
        total_usage.ru_inblock  +=  usage.ru_inblock;
        total_usage.ru_oublock  +=  usage.ru_oublock;
        total_usage.ru_msgsnd   +=  usage.ru_msgsnd;
        total_usage.ru_msgrcv   +=  usage.ru_msgrcv;
        total_usage.ru_nsignals +=  usage.ru_nsignals;
        total_usage.ru_nvcsw    +=  usage.ru_nvcsw;
        total_usage.ru_nivcsw   +=  usage.ru_nivcsw;
    } else if (setting == PRINT) {
        print_resources(total_usage);
    }
}

/**
 * prints current process's resource usage as well as current
 * runningn total
 */
void show_all_resources ()
{
    struct rusage ruse;
    manage_rusage(PRINT, ruse);
    getrusage(RUSAGE_SELF, &ruse);
    print_resources(ruse);
}

/**
 * prints the resources in the given rusage struct
 */
static void print_resources (struct rusage usage)
{
    printf("\n");
    time_t time1;
    time_t time2;
    time1 = usage.ru_utime.tv_sec;
    time2 = usage.ru_utime.tv_usec;
    printf("ru_utime    %ld.%ld\n", time1, time2);
    time1 = usage.ru_stime.tv_sec;
    time2 = usage.ru_stime.tv_usec;
    printf("ru_stime    %ld.%ld\n", time1, time2);
    printf("ru_maxrss   %ld\n",   usage.ru_maxrss);
    printf("ru_ixrss    %ld\n",    usage.ru_ixrss);
    printf("ru_idrss    %ld\n",    usage.ru_idrss);
    printf("ru_isrss    %ld\n",    usage.ru_isrss);
    printf("ru_minflt   %ld\n",   usage.ru_minflt);
    printf("ru_majflt   %ld\n",   usage.ru_majflt);
    printf("ru_nswap    %ld\n",    usage.ru_nswap);
    printf("ru_inblock  %ld\n",  usage.ru_inblock);
    printf("ru_oublock  %ld\n",  usage.ru_oublock);
    printf("ru_msgsnd   %ld\n",   usage.ru_msgsnd);
    printf("ru_msgrcv   %ld\n",   usage.ru_msgrcv);
    printf("ru_nsignals %ld\n", usage.ru_nsignals);
    printf("ru_nvcsw    %ld\n",    usage.ru_nvcsw);
    printf("ru_nivcsw   %ld\n",   usage.ru_nivcsw);
    printf("\n");
}

/**
 * prints resource usage of only the current process
 */
static void show_resources (int sig) {
    struct rusage ruse;
    getrusage(RUSAGE_SELF, &ruse);
    print_resources(ruse);
}

/**
 * prints resource usage of only child processes
 */
static void show_child_resources (int sig) {
    struct rusage ruse;
    getrusage(RUSAGE_CHILDREN, &ruse);
    print_resources(ruse);
}
