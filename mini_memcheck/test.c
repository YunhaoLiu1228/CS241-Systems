/**
 * mini_memcheck
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    void *p1 = malloc(30);
    void *p2 = malloc(40);
    void *p3 = malloc(50);
    free(p1);
    
    return 0;
}