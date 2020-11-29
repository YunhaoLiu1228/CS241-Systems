/**
 * nonstop_networking
 * CS 241 - Fall 2020
 */
#include "format.h"
#include "common.h"
#include "includes/dictionary.h"
#include "includes/vector.h"

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
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdbool.h>

#define MAX_CLIENTS 10
#define MAX_EVENTS 100
#define TIMEOUT 50000
#define OK "OK\n"
#define ERROR "ERROR\n"

typedef struct ConnectState {
	verb command;
	char server_filename[25];
    char header[1024];
    int status;
} ConnectState;

void sigpipe_handler() {}
void sigint_handler();
void setup_directory();
void setup_connection();
void setup_epoll();
void read_header(ConnectState* connection, int client_fd);
void execute_command(ConnectState* connection, int client_fd);
void execute_list(ConnectState* connection, int client_fd);
void execute_get(ConnectState* connection, int client_fd);
void execute_put(ConnectState* connection, int client_fd);
void execute_delete(ConnectState* connection, int client_fd);

static char* temp_dir_;
static char* port_;
static int epfd_;
static int sock_fd_;
static dictionary* client_dictionary_;
static dictionary* server_file_sizes_;
static vector* server_files_;


int main(int argc, char **argv) {
    
    if (argc != 2){
        print_server_usage();
        exit(1);  
    }

    // handle (ignore) sigpipe
   //signal(SIGPIPE, sigpipe_handler);       // TODO: can be SIG_IGN?
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    act.sa_flags = SA_RESTART;
    if ( sigaction(SIGPIPE, &act, NULL)) {
        perror("sigaction()");
        exit(1);
    }

    // handle sigint
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));

    sa.sa_handler = sigint_handler;
   // sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction()");
        exit(1);
    }

    // setup temporary directory
    setup_directory();
    LOG("directory setup");

    // TODO: setup other global variables
    port_ = strdup(argv[1]);
    client_dictionary_ = int_to_shallow_dictionary_create();
    server_file_sizes_ = int_to_shallow_dictionary_create();
    server_files_ = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    LOG("global variables initialized");

    // setup server connection
    setup_connection();
    LOG("connection setup");

    // setup epoll stuff
    setup_epoll();
    LOG("epoll setup");


}

// ------------------ HELPER FUNCTIONS -------------------- //

void sigint_handler() {
    // TODO: shut stuff down
    LOG("Exiting...");
    close(epfd_);
    vector_destroy(server_files_);
    dictionary_destroy(client_dictionary_);
    dictionary_destroy(server_file_sizes_);
    exit(1);
}

void setup_directory() {
    char template[] = "XXXXXX";
    temp_dir_ = mkdtemp(template);
    print_temp_directory(temp_dir_);
}

void setup_connection() {
    int getaddrinfo_res;
    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd_ == -1) {
        perror("socket()\n");
        exit(1);
    }
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo_res = getaddrinfo(NULL, port_, &hints, &result);

    if (getaddrinfo_res != 0) {
        fprintf(stderr, "%s", gai_strerror(getaddrinfo_res));
        if (result) freeaddrinfo(result);
        exit(1);
    }
     int val = 1;

    if (setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))) {
        perror("setsockopt()");
        if (result) freeaddrinfo(result);
        exit(1);
    }

    if (bind(sock_fd_, result->ai_addr, result->ai_addrlen) == -1) {
        perror("bind()");
        if (result) freeaddrinfo(result);
        exit(1);
    }
    
    if (listen(sock_fd_, MAX_CLIENTS) == -1) {
        perror("listen()");
        if (result) freeaddrinfo(result);
        exit(1);
    }
}

void setup_epoll() {
    epfd_ = epoll_create(100);
    if (epfd_ == -1) {
        perror("epoll_create()");
        exit(1);
    }
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN;  // EPOLLIN==read, EPOLLOUT==write
    ep_event.data.fd = sock_fd_;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, sock_fd_, &ep_event) == -1) {
        perror("epoll_ctl...()");
        exit(1);
    }
    struct epoll_event ep_events[MAX_EVENTS];
    
    while (true) {
        int num_fds = epoll_wait(epfd_, ep_events, MAX_EVENTS, -1);
        if (num_fds == -1) {
            perror("epoll_wait()");
            exit(1);
        } else if (num_fds == 0) {
            continue;   // keep waiting
        }

        for (int i = 0; i < num_fds; i++) {
            int ep_fd = ep_events[i].data.fd;
            // struct sockaddr addr;
            // socklen_t addrlen = sizeof(addr);
            if (ep_fd == sock_fd_) {
                int client_fd = accept(sock_fd_, NULL, NULL);  // should be NULL, NULL?
                if (client_fd == -1) {
                    perror("accept()");
                    exit(1);
                }
                struct epoll_event another_event;
                another_event.events = EPOLLIN;  // EPOLLIN==read, EPOLLOUT==write
                another_event.data.fd = client_fd;
                if (epoll_ctl(epfd_, EPOLL_CTL_ADD, client_fd, &another_event) == -1) {
                    perror("epoll_ctl!()");
                    exit(1);
                }

                ConnectState* connection = calloc(1, sizeof(ConnectState));
                connection->status = 0;
                dictionary_set(client_dictionary_, &client_fd, connection);
            } else {               
                ConnectState* client_connection = dictionary_get(client_dictionary_, &ep_fd);
                // TODO: check status stuff
                if (client_connection->status == 0) {   // haven't processed header
                    read_header(client_connection, ep_fd);
                    LOG("header read");
                } else if (client_connection->status == 1) {    // already processed header
                    execute_command(client_connection, ep_fd);
                    LOG("command executed");
                }

            }
        }
    }

}

void read_header(ConnectState* connection, int client_fd) {

    char header[1024];
    read_header_from_socket(client_fd, header, 1024);

   // printf("header: %s\n", header);
    if (strncmp(header, "LIST", 4) == 0) {
        connection->command = LIST;
    }

    else if (strncmp(header, "PUT", 3) == 0) {
        connection->command = PUT;
        strcpy(connection->server_filename, header + 4);
        printf("len: %zu\n", strlen(connection->server_filename));
       // connection->server_filename = strdup(connection->header + 4);
       // printf("filename: %s\n", connection->server_filename);

    }

    else if (strncmp(header, "GET", 3) == 0) {
        connection->command = GET;
        strcpy(connection->server_filename, header + 4);

    }

    else if (strncmp(header, "DELETE", 6) == 0) {
        connection->command = DELETE;
        strcpy(connection->server_filename, header + 7);

   
    } else {
        print_invalid_response();
        return;
    }

    if (connection->command != LIST) {
        connection->server_filename[strlen(connection->server_filename) - 1] = '\0';
    }
        connection->status = 1;

    //connection->server_filename = strdup(connection->header + strlen(connection->command));

}

void execute_command(ConnectState* connection, int client_fd) {
    verb command = connection->command;
    if (command == GET) {
        execute_get(connection, client_fd);
    }

    if (command == PUT) {
        execute_put(connection, client_fd);
        write_all_to_socket(client_fd, OK, 3);
    }

    if (command == LIST) {
        write_all_to_socket(client_fd, OK, 3);

        execute_list(connection, client_fd);
        //write_all_to_socket(client_fd, (char*) &files_size, sizeof(size_t)); 
    }

    if (command == DELETE) {
        execute_delete(connection, client_fd);
    }

    epoll_ctl(epfd_, EPOLL_CTL_DEL, client_fd, NULL);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
}

void execute_get(ConnectState* connection, int client_fd) {

}

void execute_put(ConnectState* connection, int client_fd) {

	int len = strlen(temp_dir_) + strlen(connection->server_filename) + 2;
	char file_path[len];
	memset(file_path , 0, len);
	sprintf(file_path, "%s/%s", temp_dir_, connection->server_filename);
  //  printf("file path: %s\n", file_path);
    
    FILE* read_file = fopen(file_path, "r");
    FILE* write_file = fopen(file_path, "w");

    if (!write_file) {
        perror("fopen()");
        exit(1);
    }


    // read file contents from client

    size_t buff;
    read_all_from_socket(client_fd, (char*) &buff, sizeof(size_t));
    size_t read_bytes = 0;

    while (read_bytes < buff + 4) {
        size_t header_size;
        if ((buff + 4 - read_bytes) <= 1024) {
            header_size = (buff + 4 - read_bytes);
        } else {
            header_size = 1024;
        }
        char buffer[1025];
        memset(buffer, 0, 1025);
        ssize_t read_c = read_all_from_socket(client_fd, buffer, header_size);
      //  printf("buffer: %s\n", buffer);
        if (read_c == -1) continue;

        fwrite(buffer, 1, read_c, write_file);
        read_bytes += read_c;

         if (read_c == 0) break;

    }
    
    printf("FILE NAME: %s\n", connection->server_filename);
    if (!read_file) {
        vector_push_back(server_files_, connection->server_filename);
    } else {
        fclose(read_file);
        // TODO: push back again???
    }
    fclose(write_file);
    dictionary_set(server_file_sizes_, connection->server_filename, &buff);
    
}

void execute_list(ConnectState* connection, int client_fd) {
    LOG("execute list");

    size_t size = 0;
    VECTOR_FOR_EACH(server_files_, file, {
        size += strlen(file) + 1;
    });
    if (size) size--;
//    printf("size: %zu\n", size);
    write_all_to_socket(client_fd, (char*) &size, sizeof(size_t));
    
    VECTOR_FOR_EACH(server_files_, file, {
        write_all_to_socket(client_fd, file, strlen(file));
        if (_it != _iend-1) {
            write_all_to_socket(client_fd, "\n", 1);
        }
    });
    //


}

void execute_delete(ConnectState* connection, int client_fd) {

}