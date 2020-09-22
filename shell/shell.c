/**
 * shell
 * CS 241 - Fall 2020
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

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

//static FILE* hist_fptr = NULL;


void history_write(char* command) {
    printf("command: %s\n", command);
    //printf("size: %zu\n", vector_size(history_vec));
    FILE* hist_fptr = fopen(history_file, "a");
    if(hist_fptr == NULL) {
         // there was an error
        print_script_file_error();  
        exit(1);
    }
    char* c = strdup(command);
    strcat(c, "\n");
    
    int write_status = fputs(c, hist_fptr);
    if (write_status < 0) {
        print_history_file_error();
        exit(1);
    }
    
    fclose(hist_fptr);

}


pid_t exec_external_command(char* command, bool* store_history) {
    *store_history = true;
    int status;
    pid_t pid = fork();
    pid_t childpid = pid;

    // char* com = strdup(command);
    // char* args = NULL;
    //bool multiple_args = false;
    
    if (pid < 0) {     // y i k e s
        print_fork_failed();
        exit(1);

    } else if (pid == 0) {  // CHILD:
        // ADD TO HISTORY VECTOR!
         // then exec
         //printf("com: %s, args: %s\n", com, args);
        childpid = getpid();

        char* com = strdup(command);
        char* args = NULL;
        if (strchr(command, ' ') != NULL) { 
            sstring* s = cstr_to_sstring(command);
            vector* v = sstring_split(s, ' ');

            com = vector_get(v, 0);
            args = vector_get(v, 1);

            //printf("com: %s, args: %s\n", com, args);
        } 
        if (args) {
            status = execlp(com, com, args, NULL);
        } else {
            status = execlp(com, com, NULL);
        }

        if (status < 0) {     /* execute the command  */
            print_exec_failed(command);
            exit(1);
        } 
    } else {        // PARENT:
        
        waitpid(pid, &status, 0);
        fflush(stdout);
         //print_prompt(get_full_path(command_file), pid);
    }
    //free(com);
    return childpid;
}


void exec_cd(char* command) {
    // cd ...
    if (strlen(command) < 4) {
        command += 2;
        print_no_directory(command);
        
    }

    char path[1000];
    getcwd(path, sizeof(path));

    command += 3;       // move pointer forward three chars
    strcat(path, "/");
    strcat(path, command);

    int dir = chdir(path);
    if (dir != 0) {
        print_no_directory(path);
        exit(1);
    }
}

void exec_print_history() {
    for (size_t i = 0; i < vector_size(history_vec); i++) {
        print_history_line(i, vector_get(history_vec, i));
    }

}

void exec_nth(char* command) {
    // command = #<n>
    command++;
    
    for (size_t i = 0; i < strlen(command); i++) {
        if (!isdigit(command[i])) {
            print_invalid_index();
            return;
        }
        
    }
    int n = atoi(command);
    //printf("n: %d\n", n);
    if (n < 0 || (unsigned int)n >= vector_size(history_vec)) {
        print_invalid_index();
        return;
    }
    //char* com = vector_get(history_vec, n);
    //printf("com: %s\n", com);
    //TODO: THIS DOESN'T EXECUTE ANYTHING

}


bool exec_internal_command(char* command, bool* store_history) {
    // cd
    if (command[0] == 'c' && command[1] == 'd') {
        exec_cd(command);
        return true;
    } else if (command[0] == '!') {
        //  !history
        if (strcmp(command, "!history") == 0) {
            *store_history = false;
            exec_print_history();
            return true;
        // !<prefix>
        } else {
            *store_history = false;
            return true;
        }
    //  #<n>
    } else if( command[0] == '#') {
        exec_nth(command);
        *store_history = false;
        return true;
    } else {
        return false;
    }
}


pid_t parse_external_command(char* command, bool* store_history) {
    char* c = strchr(command, ';');
    if (c != NULL)  {
        printf("%s\n", c);
    }
    return 0;
}

void execute_command(char* command) {
    if (!command ||  strlen(command) == 0) return;
    /**
    * get the current path
    **/
    char cwd[1000]; //getcwd(char *buf, size_t size);
    getcwd(cwd, sizeof(cwd));

    /**
    * print prompts IF IN -F MODE
    **/
   pid_t pid = getpid();
    if (command_flag) {
        
        fflush(stdout);
        print_prompt(cwd, pid);
        print_command(command);
    }

    /**
    * execute
    **/
    bool store_history = true;
    if (!exec_internal_command(command, &store_history)) {
        pid = exec_external_command(command, &store_history);
        //pid = parse_external_command(command, &store_history);
    }
    //("store hist: %d\n", store_history);

    /**
     * add to history vector
     **/
    if (store_history) {
        vector_push_back(history_vec, command);
        if (history_flag) {
            history_write(command);
        }
    }

    print_command_executed(pid);

    
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
                //printf("here\n");
                history_file = strdup(optarg);
                history_flag = true;
                break;

            default:    // if there are no args that's ok
                break;
        }
    }


    // now we can initialize the history vector
    history_vec = vector_create(string_copy_constructor, string_destructor, string_default_constructor); 

    if (command_flag) {

        /**
         * open the file of commands
         **/
        FILE* fptr =  fopen(command_file, "r");
        if(fptr == NULL) {
            // there was an error
            print_script_file_error();  
            exit(1);             
        }

        


        /** 
         * get number of lines
         **/
        size_t count_lines = 0;
        char chr;
        for (chr = getc(fptr); chr != EOF; chr = getc(fptr)) {
            if (chr == '\n') { // Increment count if this character is newline 
                count_lines = count_lines + 1; 
            }
        }
        fclose(fptr);       // close the file!!


        /** 
         * put all the commands from the file into myargv
         **/
        fptr = fopen(command_file, "r");    // gotta reopen it smh
        char** myargv = malloc(sizeof(char*) * count_lines); 

        char* line = malloc(sizeof(char) * 100);
        size_t len;
        ssize_t read;

        size_t line_index = 0;


        /**
         * do the actual putting
         **/
        while ((read = getline(&line, &len, fptr)) != -1) {
            myargv[line_index] = strdup(line);
            strtok(myargv[line_index], "\n");       // strip newline
            strcat(myargv[line_index], "\0");
            line_index++;
        }
        fclose(fptr);

        myargv[count_lines-1] = NULL;
        free(line);



        /**
         * now start execing stuff
         **/ 
        for (size_t i = 0; i < count_lines; i++) {
            execute_command(myargv[i]);    
        }

        


    /** if executing commands from STDIN: **/
    } else {
        pid_t pid = getpid();

        char path[1000];
        char str[1000];
        char *p;

        while (true) {
            
            if (getcwd(path, sizeof(path)) != NULL) {
                print_prompt(path, pid);
            }

            if (*str == EOF) {
                exit(0);
            }

            if (fgets(str, 1000, stdin) == NULL) {    // if EOF is encountered and no characters have been read
                printf("^D\n");           //      - this happens when ctrl-D is pressed
                exit(0);
            }

            if ((p = strchr(str, '\n')) != NULL) {
                *p = '\0';
                execute_command(str);
            }
        }
    }

   
    return 0;
}
