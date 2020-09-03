/**
 * extreme_edge_cases
 * CS 241 - Fall 2020
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * param input_str: a C string, which represents an arbitrary number of sentences
 * returns: NULL of input_str is a NULL ptr, else an array of output_s for every 
 *          input_s in the input string, terminated by a NULL pointer.
 *          output_s is the the concatenation of all words w in input_s after w has been camelCased.
 *          The punctuation from input_s is not added to output_s.
 */

char **camel_caser(const char *input_str) {
    /*
    if (input_str == NULL) return NULL;

    char** output_s = NULL;                        // the output array of char *s
    char* input_s = strdup(input_str);      // STRDUP ALLOCATES NEW MEMORY!!!
    
    while(input_s) {
        printf("%d", *input_s);
        input_s++;
    }

    return output_s;
    */
   return NULL;
}

void destroy(char **result) {
    // TODO: Implement me!
    return;
}
