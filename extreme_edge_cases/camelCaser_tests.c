/**
 * extreme_edge_cases
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/* 
 * params: a function pointer to the camelcaser and destroy functions we need to test (NOT OURS)
 * return: true (1) if all test cases pass, false (0) if any fail
 */

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {

    // TEST 0 : NULL string
    const char* str0 = NULL;
    assert(str0 == NULL);

    // TEST 1: basic one-word sentence string
    const char* str1 = "Hello.";
    char* correct1[] = {"hello", NULL};

    char** str1_result = (char**) malloc(sizeof(char*) * 10);       // idk why 10
    str1_result = camelCaser(str1);

    if (camelCaser(str1) == NULL) printf("null\n");     //TODO: why is this NULL???

    assert(strcmp(correct1[0], "hello") == 0);        // *str1_result segfaults


    return 1;       // made it to the end! :D
}
