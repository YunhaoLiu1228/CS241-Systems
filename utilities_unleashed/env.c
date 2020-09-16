/**
 * utilities_unleashed
 * CS 241 - Fall 2020
 */
#include "format.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc <= 2 || !argv) {
        print_env_usage();
    }

    // for(int i = 0; i < argc; i++) {
    //     printf("%s\n", argv[i]);
    // }
    // printf("--------------\n");

    char** child_argv = malloc(sizeof(char*) * argc);   // put the args from argv in 

    pid_t pid = fork();

    if (pid < 0) {          // fork failure
        print_fork_failed();
        exit(1);

    } else if (pid > 0) {   // parent
        int status;
        waitpid(pid, &status, 0);

    } else {                // child
        // SET THE CHILD ARGV ARRAY
        for (int idx = 0; idx < argc -1; idx++) {
            child_argv[idx] = argv[idx + 1];
        }
        child_argv[argc - 2] = argv[argc-1];

        // GET THE KEY / VALUE PAIRS - only works for one env variable
        char* key = strtok(child_argv[0], "=");
        char* value = strtok(NULL, "=");
        //}
        setenv(key, value, 1);

        // for(int i = 0; i < argc-1; i++) {
        //     printf("%s\n", child_argv[i]);
        // }
        // printf("--------------\n");
    
        //setenv(1);
        execvp(child_argv[argc-2], &child_argv[argc-2]);

        exit(1); // For safety. 
    }

    free(child_argv);
    return 0;
}
