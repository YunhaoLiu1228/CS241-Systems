/**
 * nonstop_networking
 * CS 241 - Fall 2020
 */
#include "format.h"
#include "common.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>


char **parse_args(int argc, char **argv);
verb check_args(char **my_args);

// my functions
int connect_to_server(char* host, char* port);
int execute_request(verb request);
int write_client_request(verb request);
int handle_put();

static char** my_args;
static int sock_fd;

// TODO: use format.c error prints !!!!! ***** ~~~~~~ !!!!!
int main(int argc, char **argv) {

    // parse my my_args into string
    // Returns char* array in form of:
    // 0: host, 
    // 1: port, 
    // 2: request (method),
    // 3: remote, 
    // 4: local, 
    // 5: NULL
    my_args = parse_args(argc, argv);

    // convert string to verb enum
    verb request = check_args(my_args);

    // if an unknown
    if (request == V_UNKNOWN) {
        exit(1);
    }

    // make connection to server
    sock_fd = connect_to_server(my_args[0], my_args[1]);
    if (sock_fd == 1) {
        exit(1);
    }

    // write client request to stdout
    if (write_client_request(request) == 1) {
        exit(1);
    }

    // execute the request
    if (execute_request(request) == 1) {
        exit(1);
    }

    // yay!
   // print_success();

    // cleanup
    shutdown(sock_fd, SHUT_RD);

    close(sock_fd);
    free(my_args);
    return 0;

}

// set up connection to the sever
// returns sock_fd on success, 1 on failure
int connect_to_server(char* host, char* port) {

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int get_addr_info = getaddrinfo(host, port, &hints, &result);

    if (get_addr_info) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_addr_info));
        return 1;
    }

    int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd == -1) {
        perror("socket()");
        return 1;
    }

    int connection = connect(sock_fd, result->ai_addr, result->ai_addrlen);
    if (connection == -1) {
        perror("connect()");
        return 1;
    } 

    freeaddrinfo(result);

    // yay!!
    return sock_fd;
}

/** my_args is a char* array in form of:
    0: host, 
    1: port, 
    2: request, 
    3: remote, 
    4: local, 
    5: NULL
    returns 0 on success, 1 on failure
**/
int write_client_request(verb request) {

    char* s;
    // LIST\n
    if (request == LIST) {
        s = malloc(strlen(my_args[2]) + 2);
        sprintf(s, "%s\n", my_args[2]);
    }

    // GET remote\n     ~or~    PUT remote/n     ~or~       DELETE remote/n 
    else {
        size_t total_len = strlen(my_args[2])+strlen(my_args[3])+3;
        s = malloc(total_len);
        sprintf(s, "%s %s\n", my_args[2], my_args[3]);
    }
    int write_count = write_all_to_socket(sock_fd, s, (ssize_t)strlen(s));
    if (write_count < (ssize_t)strlen(s)) {
        print_connection_closed();
        // >:(
        return 1;
    }

    free(s);
    return 0;
}



int execute_request(verb request) {

    // upload data if request is PUT
    if (request == PUT) {
        handle_put();
    }
    
    // shut down write half
    if (shutdown(sock_fd, SHUT_WR) != 0) {
       perror("shutdown()");
       return 1;
    }
    char* OK = "OK\n";
    char* ERROR = "ERROR\n";
    char* buffer = calloc(1, strlen(OK) + 1);     // need +1 for \0 !!!!

    size_t read_bytes = read_all_from_socket(sock_fd, buffer, strlen(OK));
 
    if (strcmp(buffer, OK) != 0) {
        char* new_buffer = realloc(buffer, strlen(ERROR) + 1);
        read_all_from_socket(sock_fd, new_buffer + read_bytes, strlen(ERROR) - read_bytes);
        
        if (strcmp(new_buffer, ERROR) == 0) {
            //fprintf(stdout, "%s", new_buffer);
            char error_message[24] = {0};

            if (read_all_from_socket(sock_fd, error_message, 24) == 0) {
                print_connection_closed();
            }

            print_error_message(error_message);

        } else {
            print_invalid_response();
        }
        return 1;
    }
    
    // server response is OK!!
    //fprintf(stdout, "%s", buffer);
    // now start fulfilling requests

    // --- LIST ---
    if (request == LIST) {
            size_t size;
            read_all_from_socket(sock_fd, (char*)&size, sizeof(size_t));
            char buffer[size + 6];
            memset(buffer, 0, size + 6);
            read_bytes = read_all_from_socket(sock_fd, buffer, size + 5);
            //error detect
            if (read_bytes == 0 && read_bytes != size) {
                print_connection_closed();
                return 1;
            } else if (read_bytes < size) {
                print_too_little_data();
                return 1;
            } else if (read_bytes > size) {
                print_received_too_much_data();
                return 1;
            }
            fprintf(stdout, "%s\n", buffer);       // TODO: newline here ok? also no space between size and buffer?
    }

    // --- GET ---
    else if (request == GET) {
        FILE *local_file = fopen(my_args[4], "a+");
        if (!local_file) {
            perror("fopen()");
            return 1;
        }
        size_t buff_size;
        read_all_from_socket(sock_fd, (char *)&buff_size, sizeof(size_t));
        size_t read_bytes = 0;
        size_t r_size;

        while (read_bytes < buff_size + 4) {
            if ((buff_size + 4 - read_bytes) > 1024){
                r_size = 1024;
            }else{
                r_size = buff_size + 4 - read_bytes;
            }
            char buffer[1025] = {0};
            size_t read_count = read_all_from_socket(sock_fd, buffer, r_size);
            fwrite(buffer, 1, read_count, local_file);
            read_bytes += read_count;

            if (read_count == 0) {
                break;
            }
        }

        if (read_bytes < buff_size) {
            print_too_little_data();
            return 1;
        } else if (read_bytes > buff_size) {
            print_received_too_much_data();
            return 1;
        } else if (read_bytes == 0 && read_bytes != buff_size) {
            print_connection_closed();
            return 1;
        } 


        fclose(local_file);
    }

    // --- PUT ---
    else if (request == PUT) {
        print_success();
    }

    // --- REQUEST ---
    else if (request == DELETE) {
        print_success();
    }

    free(buffer);
    return 0;
}


int handle_put(){
    struct stat statbuf;

    if(stat(my_args[4], &statbuf) == -1) {
        return 1;
    }

    size_t stat_size = statbuf.st_size;
    write_all_to_socket(sock_fd, (char*)&stat_size, sizeof(size_t));

    // write data
    FILE* local_file = fopen(my_args[4], "r");
        
    if(!local_file) {
        return 1;
    }

    ssize_t w_count;
    size_t w_total = 0;

    while (w_total < stat_size) {
        if ((stat_size - w_total) <= 1024 ){
            w_count = stat_size - w_total;
        } else {
            w_count = 1024;
        }

        char buffer[w_count + 1];
        fread(buffer, 1, w_count, local_file);

        //int read_bytes = fread(buffer, 1, w_count, local_file);
        // if (read_bytes == 0) {
        //     break;
        // }
        ssize_t write_count = write_all_to_socket(sock_fd, buffer, w_count);
        if ( write_count < w_count) {
            print_connection_closed();
            return 1;
        }

        w_total += w_count;
    }
    // close file
    fclose(local_file);
    return 0;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **my_args = calloc(1, 6 * sizeof(char *));
    my_args[0] = host;
    my_args[1] = port;
    my_args[2] = argv[2];
    char *temp = my_args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        my_args[3] = argv[3];
    }
    if (argc > 4) {
        my_args[4] = argv[4];
    }

    return my_args;
}

/**
 * Validates my_args to program.  If `my_args` are not valid, help information for the
 * program is printed.
 *
 * my_args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **my_args) {
    if (my_args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = my_args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (my_args[3] != NULL && my_args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (my_args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (my_args[3] == NULL || my_args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}


