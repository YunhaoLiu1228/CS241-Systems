/**
 * charming_chatroom
 * CS 241 - Fall 2020
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    int32_t message_size = htonl(size);

    ssize_t write_bytes = write_all_to_socket(socket, (char*) &message_size, MESSAGE_SIZE_DIGITS);

    return write_bytes;
}

/**
 
while number of bytes is not the number needed:
    return_code = read bytes from socket
    if return_code = 0:
        return bytes read
    else if return_code > 0:
        add return_code bytes to counter
    else if return_code == -1 and error was interrupted:
        try again
    else:
        return -1
return bytes read

**/
ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    ssize_t read_bytes = 0;
    ssize_t remaining_bytes = count;
    ssize_t return_code;

    while (read_bytes != 0 ) {
        return_code = read(socket, (void*) (buffer + read_bytes), remaining_bytes);

        if (return_code == 0) {
            return read_bytes;

        } else if (return_code > 0) {
            read_bytes += return_code;
            remaining_bytes -= return_code;
        } else if (return_code == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }
    return read_bytes;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {

    ssize_t read_bytes = 0;
    ssize_t remaining_bytes = count;
    ssize_t return_code;

    while (read_bytes != 0 ) {
        return_code = write(socket, (void*) (buffer + read_bytes), remaining_bytes);

        if (return_code == 0) {
            return read_bytes;

        } else if (return_code > 0) {
            read_bytes += return_code;
            remaining_bytes -= return_code;

        } else if (return_code == -1 && errno == EINTR) {
            continue;

        } else {
            return -1;
            
        }
    }
    return read_bytes;
}
