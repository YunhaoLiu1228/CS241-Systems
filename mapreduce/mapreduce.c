/**
 * mapreduce
 * CS 241 - Fall 2020
 */
#include "utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


/**
    * Split the input file into <mapper_count> parts and pipe the contents into <mapper_count> different mapper processes (use splitter).
    * Pipe the output of the mapper processes into the reducer process
    * Write the output of the reducer process to the output file.
    * Parallelize these tasks to achieve speedup
*/

int main(int argc, char **argv) {
    
    // input form:  ./mapreduce <input_file> <output_file> <mapper_executable> <reducer_executable> <mapper_count>
    // example:     ./mapreduce test.in test.out ./my_mapper ./my_reducer 3

    if (argc != 6) {
        print_usage();
        return 1;
    }

    if (atoi(argv[5]) < 1) {
        print_usage();
        return 1;
    }

    char* input_file = strdup(argv[1]);
    char* output_file = strdup(argv[2]);
    char* mapper_exec = strdup(argv[3]);
    char* reducer_exec = strdup(argv[4]);
    int mapper_count = atoi(argv[5]);

    // Create an input pipe for each mapper.
    int* input_fd[mapper_count];

    for (int i = 0; i < mapper_count; i++) {
        input_fd[i] = malloc(2 * sizeof(int));
        pipe(input_fd[i]);
    } 

    // Create one input pipe for the reducer.
    int reducer_fd[2];
    pipe (reducer_fd);

    // Open the output file.
    int output_fd = open(output_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if ( output_fd == -1) {
        return 1;
    }   

    // Start a splitter process for each mapper.
    // "start up one instance of splitter for each mapper, using a pipe to
    //  send stdout of splitter to stdin of the mapper program."

    // splitter usage: ./splitter <input_file> <count> <index>
    pid_t pids[mapper_count];
    
    for (int i = 0; i < mapper_count; i++) {
        pid_t child = fork();
        pids[1] = child;
        if (child > 0) { /* I must be the parent */
            close(input_fd[i][0]);
            dup2(input_fd[i][1], 1);
            execl("./splitter", "./splitter", input_file, mapper_count, i);

            return 1; // this is bad
        }
    }
    // Start all the mapper processes.
    pid_t child_m[mapper_count];

    for (int i = 0; i < mapper_count; i++) {
        close(input_fd[i][1]);
        pid_t child = fork();
        child_m[i] = child;
        if (!child) {
            close(reducer_fd[0]);
            dup2(input_fd[i][0], 0);
            dup2(reducer_fd[1], 1);
            execl(mapper_exec, mapper_exec, NULL);
            
            return 1;
        }
    }

    // Start the reducer process
    close(reducer_fd[1]);
    pid_t child = fork();
    if (child == 0) {
        dup2(reducer_fd[0], 0);
        dup2(output_fd, 1);
        execl(reducer_exec, reducer_exec, NULL);
        exit(1);
    }
    close(reducer_fd[0]);
    close(output_fd);

    // Wait for the reducer to finish.
    for (int i = 0; i < mapper_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    
    for (int i = 0; i < mapper_count; i++) {
        close(input_fd[i][0]);
        int status;
        waitpid(child_m[i], &status, 0);
    }

    int status;
    waitpid(child, &status, 0);

    // Print nonzero subprocess exit codes.
    if (status) print_nonzero_exit_status(reducer_exec, status);


    // Count the number of lines in the output file.
    print_num_lines(output_file);

    for (int i = 0; i < mapper_count; i++) {
        free(input_fd[i]);
    }

    free(input_file);
    input_file = NULL;
    free(output_file);
    output_file = NULL;
    free(mapper_exec);
    mapper_exec = NULL;
    free(reducer_exec);
    reducer_exec = NULL;

    return 0;
}
