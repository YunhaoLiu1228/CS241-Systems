/**
 * utilities_unleashed
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "format.h"

#define BILLION 1000000000L

int main(int argc, char *argv[]) {
    if (argc <= 1 || !argv) {
        print_time_usage();     // prints correct usage and exits
    }

    struct timespec start, end;     // use these to calculate the execution time
    pid_t pid = fork();
    int status;
    char** child_argv = malloc(sizeof(char*) * argc);   // put the args from argv in 

    if (pid < 0) {          // Y I K E S
        print_fork_failed();
        exit(1);  

    } else if (pid > 0) {   // parent
        clock_gettime(CLOCK_MONOTONIC, &start);
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);

    } else {                // child
        for (int idx = 0; idx < argc - 1; idx++) {
            child_argv[idx] = argv[idx + 1];
        }
        child_argv[argc - 1] = NULL;

        status = execvp(*child_argv, child_argv);
        if (status != 0) {
            print_exec_failed();
        }
        exit(1);
     }
    double time =  ( (end.tv_sec - start.tv_sec)*BILLION + end.tv_nsec - start.tv_nsec) / BILLION;

    free(child_argv);
    if (status == 0) {
        display_results(argv, time);
    }
    
    return 0;
}
