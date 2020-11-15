/**
 * nonstop_networking
 * CS 241 - Fall 2020
 */
#include "common.h"


ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    ssize_t read_bytes = 0;
    ssize_t remaining_bytes = count;
    ssize_t return_code;

    while (remaining_bytes > 0  || (return_code == -1 && errno == EINTR)) {
        return_code = read(socket, (void*) (buffer + read_bytes), remaining_bytes);

        if (return_code == 0) {
            return 0;

        } else if (return_code > 0) {
            read_bytes += return_code;
            remaining_bytes -= return_code;
        } 

    }
    return read_bytes;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {

    ssize_t written_bytes = 0;
    ssize_t remaining_bytes = count;
    ssize_t return_code;

    while (remaining_bytes > 0  || (return_code == -1 && errno == EINTR)) {
        return_code = write(socket, (void*) (buffer + written_bytes), remaining_bytes);

        if (return_code == 0) {
            return 0;

        } else if (return_code > 0) {
            written_bytes += return_code;
            remaining_bytes -= return_code;
        } 

    }
    return written_bytes;
}