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
    int status;

    if (pid < 0) {          // fork failure
        print_fork_failed();
        exit(1);

    } else if (pid > 0) {   // parent
        
        waitpid(pid, &status, 0);

    } else {                // child
        // SET THE CHILD ARGV ARRAY
        for (int idx = 0; idx < argc -1; idx++) {
            child_argv[idx] = argv[idx + 1];
        }
        child_argv[argc - 2] = argv[argc-1];

            // GET THE KEY / VALUE PAIRS - only works for one env variable
        if (argc == 4) {
            char* key = strtok(child_argv[0], "=");
            char* value = strtok(NULL, "=");

            int env_change = setenv(key, value, 1);
            if (env_change!=0) {
                free(child_argv);
                print_environment_change_failed();
            }
        } else {        // ONLY WORKS IF ARG = 5
            char* key = strtok(child_argv[0], "=");
            char* value = strtok(NULL, "=");
        

            char* key2 = strtok(child_argv[1], "=%");
            char* value2 = strtok(NULL, "=%");


            int env_change;
            if (strcmp(key, value2) == 0) {
                env_change = setenv(key2, value, 1);
            } else {
                env_change = setenv(key, value, 1);
            }

            
            if (env_change!=0) {
                free(child_argv);
                print_environment_change_failed();
            }

            // int env_change = setenv(key, value, 1);
            // if (env_change!=0) {
            //     free(child_argv);
            //     print_environment_change_failed();
            // }

        }
    
        status = execvp(child_argv[argc-2], &child_argv[argc-2]);

        exit(1); // For safety. 
    }

    if (status != 0) {
        free(child_argv);
        print_exec_failed();
    }
    free(child_argv);
    return 0;
}
