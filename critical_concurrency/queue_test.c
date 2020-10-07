/**
 * critical_concurrency
 * CS 241 - Fall 2020
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s test_number return_code\n", argv[0]);
        exit(1);
    }
    queue* q = queue_create(10);

    int in[10] = {1, 2, -3, -4, 5, 60, -70, 8, -9, 1000};

    for (int i = 0; i < 10; i++) {
        printf("pushing: %d\n", in[i]);
        queue_push(q, &in[i]);
        usleep(500000);
    }
    
    puts("- - - - - - - - - - - - - - - -");

    for (int i = 0; i < 10; i++) {
        printf("pulling: %d\n", *(int*)queue_pull(q));
        usleep(500000);
    }

    queue_destroy(q);
    return 0;
}
