/**
 * nonstop_networking
 * CS 241 - Fall 2020
 */
#include "format.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "common.h"

char **parse_args(int argc, char **argv);
verb check_args(char **args);

// my functions
int connect_to_server(char* host, char* port);
int execute_request(verb request);
int write_client_request(char** args, verb request, int sock_fd);

// taken from my chatroom lab :)
ssize_t read_all_from_socket(int socket, char *buffer, size_t count);
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);


// TODO: use format.c error prints !!!!! ***** ~~~~~~ !!!!!
int main(int argc, char **argv) {

    // parse my args into string
    // Returns char* array in form of:
    // 0: host, 
    // 1: port, 
    // 2: request (method),
    // 3: remote, 
    // 4: local, 
    // 5: NULL
    char** my_args = parse_args(argc, argv);

    // convert string to verb enum
    verb request = check_args(my_args);

    // if an unknown
    if (request == V_UNKNOWN) {
        exit(1);
    }

    // make connection to server
    int sock_fd;
    if ((sock_fd = connect_to_server(my_args[0], my_args[1]) == 1)) {
        exit(1);
    }

    // write client request to stdout
    if (write_client_request(my_args, request, sock_fd) == 1) {
        exit(1);
    }

    // execute the request
    if (execute_request(request) == 1) {
        exit(1);
    }

    // yay!
    print_success();

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

/** args is a char* array in form of:
    0: host, 
    1: port, 
    2: request, 
    3: remote, 
    4: local, 
    5: NULL
    returns 0 on success, 1 on failure
**/
int write_client_request(char** args, verb request, int sock_fd) {

    char* s;
    // LIST\n
    if (request == LIST) {
        s = malloc(strlen(args[2]) + 2);
        sprintf(s, "%s\n", args[2]);
    }

    // GET remote\n     ~or~    PUT remote/n     ~or~       DELETE remote/n 
    else if (request == GET || request == PUT || request == DELETE) {
        size_t total_len = strlen(args[2]) + strlen(args[3]);
        s = malloc(total_len + 3);
        sprintf(s, "%s %s\n", args[2], args[3]);

        return 0;
    }


    if (write_all_to_socket(sock_fd, s, strlen(s)) != (ssize_t)strlen(s)){
        print_connection_closed();
        // >:(
        return 1;
    }

    free(s);
    return 0;
}



int execute_request(verb request) {
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

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}


// returns bytes read on success, 1 on error
ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    size_t return_code = 0;

    while (return_code < count) {

        ssize_t read_code = read(socket, (void*) (buffer + return_code), count - return_code);
        if (read_code == -1 && errno == EINTR) {
            continue;
        }

        if (read_code == 0) {
            break;
        }

        if (read_code == -1) {
            return 1;
        }
        return_code += read_code;
    }

    return return_code;
}


ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    size_t return_code = 0;

    while (return_code < count) {

        ssize_t write_code = write(socket, (void*) (buffer + return_code), count - return_code);
        
        if (write_code == -1 && errno == EINTR) {
            continue;
        }

        if (write_code == 0) {
            break;
        }

        if (write_code == -1) {
            return 1;
        }
        return_code += write_code;
    }

    return return_code;
}

