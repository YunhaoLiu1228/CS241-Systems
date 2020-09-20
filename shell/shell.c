/**
 * shell
 * CS 241 - Fall 2020
 */
#include "format.h"
#include "shell.h"
#include "vector.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

/** GLOBAL: VARIABLES: **/
static char* command_file = NULL;
static char* history_file = NULL;
// if history_file is specified, each subsequent command is pushed to the vector and
// also written to the file
// if not, just push command to the vector
static vector* history_vec;

static bool history_flag = false;   // start false
static bool command_flag = false;   //

void print_command_prompt(pid_t pid, char* path) {
    printf("(pid=%d)%s$ ", pid, path);
}


int shell(int argc, char *argv[]) {
    /**
     *      Starting up a shell
     *      Optional arguments when launching shell
     * Interaction
     * Built-in commands
     * Foreground external commands
     * Logical operators
     * SIGINT handling
     * Exiting
    **/

   /** FIRST: parse argv[] 
    * can be in the following formats:
    * ./shell
    * ./shell -f filename
    * ./shell -h filename
    * ./shell -h filename1 ./f filename2
    * */
   if (argc != 1 && argc != 3 && argc!= 5) {
       printf("%d\n", argc);
       // print usage
       print_usage();
       // exit TODO:
       exit(0);
   }

    /** SECOND: read args to determine optional arguments (if any) using getopt()     **/
    int c;
    while ((c = getopt(argc, argv, "h:f:")) != -1) {
        switch (c) {
            case 'f':
                command_file = strdup(optarg);
                command_flag = true;
                break;

            case 'h':
                printf("here\n");
                history_file = strdup(optarg);
                history_flag = true;
                break;

            default:    // if there are no args that's ok
                break;
        }
    }

    printf("history: %s \n", history_file);
    printf("file: %s \n", command_file);

    // now we can initialize the history vector
    history_vec = vector_create(char_copy_constructor, char_destructor, char_default_constructor); 

    if (command_flag) {
        // open the file of commands
        FILE* fptr =  fopen(command_file, "r");

        if(fptr == NULL) {
            // there was an error
            print_script_file_error();  
            exit(1);             
        }


        // get the current path
        char cwd[1024]; //getcwd(char *buf, size_t size);
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);




        //get number of lines
        size_t count_lines = 0;
        char chr;
        for (chr = getc(fptr); chr != EOF; chr = getc(fptr)) {
            if (chr == '\n') { // Increment count if this character is newline 
                count_lines = count_lines + 1; 
            }
        }

        printf("count lines: %zu\n", count_lines);
        fclose(fptr);       // close the file!!




        /** 
         * put all the commands from the file into myargv
         **/
        fptr = fopen(command_file, "r");    // gotta reopen it smh
        char** myargv = malloc(sizeof(char*) * count_lines); 

        char* line;
        size_t len;
        ssize_t read;

        size_t line_index = 0;
        // do the actual putting
        while ((read = getline(&line, &len, fptr)) != -1) {
            myargv[line_index] = strdup(line);
            line_index++;
        }
        fclose(fptr);

        myargv[count_lines-1] = NULL;




        /**
         * now start execing stuff
         **/ 
        int status = execvp(*myargv, myargv);

        if (status < 0) {
            printf("failed to exec\n");
            exit(1);
        }

            //printf("Retrieved line of length %zu:\n", read);
            //printf("%s", line);
            //char* line = "/home/eroller2/eroller2/shell/ls\0";

            // int status = execvp(myargv[0], myargv);

            // printf("here\n");
            // if (status < 0) {     /* execute the command  */
            //    printf("*** ERROR: exec failed %d\n", status);
            //    exit(1);
            // }

        //     int status;
        //     pid_t pid = fork();

        //     if (pid < 0) {          // Y I K E S & exit
        //         print_fork_failed();
        //         exit(1); 

        //     } else if (pid > 0) {   // parent
        //         waitpid(pid, &status, 0);

        //     } else {                // child
        //         execvp(line, &line);
        //         exit(1);
        //     }
        //}


    } else {

        printf("not executing from file\n");




    }

   
    return 0;
}
