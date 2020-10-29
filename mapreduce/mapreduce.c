/**
 * mapreduce Lab
 * CS 241 - Fall 2020
 */

// partner: joowonk2

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    size_t mapper_count = atoi(argv[5]);

    if (argc != 6 || mapper_count == 0) {
        print_usage();
        return 1;
    }

    char* input_file = strdup(argv[1]);
    char* output_file = strdup(argv[2]);
    char* mapper_exec = strdup(argv[3]);
    char* reducer_exec = strdup(argv[4]);
    
    
    // Create an input pipe for each mapper.
    int* fd_mappers[mapper_count];

    for (size_t i = 0; i < mapper_count; i++) {
        fd_mappers[i] = malloc(sizeof(int) * 2);
        if (pipe(fd_mappers[i]) != 0) return 1;     // bad
    }

    // Create one input pipe for the reducer.
    int fd_reducer[2];
    if (pipe(fd_reducer) != 0) return 1;        // bad!

    // Open the output file.
    int fd_output = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if (fd_output == -1) return 1;  //bad

    pid_t pids[mapper_count];
    // Start a splitter process for each mapper.
    const char* path = "./splitter";

    for (size_t i = 0; i < mapper_count; i++) {
        pid_t child = fork();
        pids[i] = child;

        if (!child) {
            char i_str[5];
            sprintf(i_str, "%zu", i);   // need to convert i to string
            dup2(fd_mappers[i][1], 1);
            close(fd_mappers[i][0]);

            execl(path, path, input_file, argv[5], i_str, NULL);
            
            return 1;       // bad!
        }
    }

    // Start all the mapper processes.
    pid_t mapper_pids[mapper_count];

    for (size_t i = 0; i < mapper_count; i++) {
        pid_t child = fork();
        mapper_pids[i] = child;

        close(fd_mappers[i][1]);
        
        if (!child) {
            close(fd_reducer[0]);
            dup2(fd_mappers[i][0], 0);
            dup2(fd_reducer[1], 1);

            execl(mapper_exec, mapper_exec, NULL);
            return 1;
        }
    }

    // Start the reducer process.
    close(fd_reducer[1]);
    pid_t child = fork();

    if (!child) {
        dup2(fd_reducer[0], 0);
        dup2(fd_output, 1);

        execl(reducer_exec, reducer_exec, NULL);
        return 1;
    }


    // Wait for the reducer to finish.
    for (size_t i = 0; i < mapper_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    for (size_t i = 0; i < mapper_count; i++) {
        close(fd_mappers[i][0]);
        int status;
        waitpid(mapper_pids[i], &status, 0);
    }


    // Print nonzero subprocess exit codes.
    int status;
    waitpid(child, &status, 0);
    if (status) print_nonzero_exit_status(reducer_exec, status);

    // Count the number of lines in the output file.
    print_num_lines(output_file);

    close(fd_reducer[0]);
    close(fd_output);

    // cleanup!!
    free(input_file);
    input_file = NULL;
    free(output_file);
    output_file = NULL;
    free(mapper_exec);
    mapper_exec = NULL;
    free(reducer_exec);
    reducer_exec = NULL;

    for (size_t i = 0; i < mapper_count; i++) {
        free(fd_mappers[i]);
    }

    return 0;   // :D
}