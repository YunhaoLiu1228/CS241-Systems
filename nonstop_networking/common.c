/**
 * nonstop_networking
 * CS 241 - Fall 2020
 */
#include "common.h"


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
