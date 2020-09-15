/**
 * vector
 * CS 241 - Fall 2020
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>


size_t sstr_length(sstring* this) {
    return this->length;
}

sstring *cstr_to_sstring(const char *input) {
    assert(input);
    sstring* sstr = calloc(1, sizeof(sstring));
    sstr->length = strlen(input);
    sstr->string = char_vector_create();
    
    char* c = strdup(input);
    for(size_t i = 0; i < strlen(input); i++) {
        vector_push_back(sstr->string, c);
        c++;
    }

    return sstr;
}

char *sstring_to_cstr(sstring *input) {
    assert(input);
    char* result = malloc(sizeof(char) * vector_size(input->string) + 1);
   // result = (char*)vector_at(input->string, 0);
    result[0] = '\0';
    strcat(result, *vector_at(input->string, 0));
    return result;
}

int sstring_append(sstring *this, sstring *addition) {
    for (size_t i = 0; i < vector_size(addition->string); i++) {
      vector_push_back(this->string, vector_get(addition->string, i));
    }
    
    this->length = (size_t)(this->length) + (addition->length);
    return this->length;
}

vector* sstring_split(sstring *this, char delimiter) {
    assert(this);

    vector* substrs = char_vector_create();
    char* curr = malloc(sizeof(char*) * vector_size(this->string));
    *curr = '\0';
    size_t first = 0;
    
    for (size_t i = 0; i < this->length; i++) {
        strncat(curr, *(char**)vector_at(this->string, i), 1);
        printf("curr: %s\n", curr);
        if (**(char**)vector_at(this->string, i) == delimiter) {
        
            vector_push_back(substrs, curr);


            first = i +1;
        }
    }
    return substrs;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    return -1;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    return NULL;
}

void sstring_destroy(sstring *this) {
    // your code goes here
}
