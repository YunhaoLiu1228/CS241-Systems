/**
 * password_cracker
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crypt.h>


#include "cracker1.h"
#include "format.h"
#include "utils.h"

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    char* line = NULL;
    size_t len;

    ssize_t read;
    while (( read = getline(&line,&len, stdin)) != -1) {
        //printf("line: %s\n", line);

        char* username = strtok(line, " ");
        char* hashed = strtok(NULL, " ");
        char* clue = strtok(NULL, " ");
        printf("user: %s, hashed: %s, clue: %s\n", username, hashed, clue);

        struct crypt_data cdata;
        cdata.initialized = 0;

        const char *ex = crypt_r(clue, "xx", &cdata);
        printf("hash of 'example1' = %s\n", ex);

    }

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
