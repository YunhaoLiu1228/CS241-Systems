/**
 * vector
 * CS 241 - Fall 2020
 */
#include "vector.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // test creating a vector
    vector* vec0 = int_vector_create();
    assert(vec0);
    int i0 = 1;
    int i1 = 7;
    int i2 = 0;
    printf("Pushing back %d\n", i0);
    vector_push_back(vec0, &i0);
    printf("Pushing back %d\n", i1);
    vector_push_back(vec0, &i1);
    printf("Pushing back %d\n", i2);
    vector_push_back(vec0, &i2);

    printf("size: %lo\n", vector_size(vec0));

    for (size_t i = 0; i < vector_size(vec0); i++) printf("%zu: %d\n", i, **((int**)vector_at(vec0, i)));
    
    return 0;
}
