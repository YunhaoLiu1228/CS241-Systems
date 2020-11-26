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

typedef struct ConnectState {
	verb command;
	char filename[256];
    int status;
} ConnectState;

void sigint_handler();
void setup_directory();
void setup_connection();
void setup_epoll();
void read_header(ConnectState* connection, int client_fd);

static char* temp_dir_;
static char* port_;
static int epfd_;
static int sock_fd_;
static dictionary* client_dictionary_;


int main(int argc, char **argv) {
    
    if (argc != 2){
        print_server_usage();
        exit(1);  
    }

    // handle (ignore) sigpipe
    signal(SIGPIPE, SIG_IGN);

    // handle sigint
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction()");
        exit(1);
    }

    // setup temporary directory
    setup_directory();

    // TODO: setup other global variables
    port_ = strdup(argv[1]);
    client_dictionary_ = int_to_shallow_dictionary_create();


    // setup server connection
    setup_connection();

    // setup epoll stuff
    setup_epoll();


}

// ------------------ HELPER FUNCTIONS -------------------- //

void sigint_handler() {
    // TODO: shut stuff down
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

    if (setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val))) {
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

                ConnectState* connection = malloc(sizeof(ConnectState));
                connection->status = 0;
                dictionary_set(client_dictionary_, &client_fd, connection);
            } else {               
                ConnectState* client_connection = dictionary_get(client_dictionary_, &ep_fd);
                // TODO: check status stuff
                if (client_connection->status == 0) {   // haven't processed header
                    read_header(client_connection, ep_fd);
                } else if (client_connection->status == 1) {    // already processed header

                }

            }
        }
    }

}

void read_header(ConnectState* connection, int client_fd) {
    char header[1024];
    ssize_t count = read_header_from_socket(client_fd, header, 1024);
    printf("count: %zu\n", count);
    printf("header: %s\n", header);
}

