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
} ConnectState;

void sigint_handler();
void setup_directory();
void setup_connection();
void setup_epoll();

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
    epfd_ = epoll_create(1);
    if (epfd_ == -1) {
        perror("epoll_create()");
        exit(1);
    }
    struct epoll_event event;
    event.events = EPOLLIN;  // EPOLLIN==read, EPOLLOUT==write
    event.data.fd = sock_fd_;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, sock_fd_, &event) == -1) {
        perror("epoll_ctl()");
        exit(1);
    }
    struct epoll_event events[MAX_EVENTS];
    
    while (true) {
        int num_fds = epoll_wait(epfd_, events, MAX_EVENTS, -1);
        if (num_fds == -1) {
            perror("epoll_wait()");
            exit(1);
        } else if (num_fds == 0) {
            continue;   // keep waiting
        }

        for (int i = 0; i < num_fds; i++) {

            struct sockaddr addr;
            socklen_t addrlen = sizeof(addr);

            if (events[i].data.fd == sock_fd_) {
                int client_fd = accept(sock_fd_, (struct sockaddr *) &addr, &addrlen);  // should be NULL, NULL?
                if (client_fd == -1) {
                    perror("accept()");
                    exit(1);
                }
                struct epoll_event another_event;
                another_event.events = EPOLLIN;  // EPOLLIN==read, EPOLLOUT==write
                another_event.data.fd = sock_fd_;
                if (epoll_ctl(epfd_, EPOLL_CTL_ADD, sock_fd_, &another_event) == -1) {
                    perror("epoll_ctl()");
                    exit(1);
                }

                ConnectState* connection = malloc(sizeof(ConnectState));
                dictionary_set(client_dictionary_, &client_fd, connection);
            } else {
                
//                ConnectState* client_connection = dictionary_get(client_dictionary_, &(events[i].data.fd));

            }
        }
    }

}

