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

static queue* q;

void* generator(void* arg) {
    int in[10] = {1,2,3,4,5,6,7,8,9,0};
    for (int i = 0; i < 10; i++) {
        queue_push(q, &in[i]);
        printf("pushing: %d\n", in[i]);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0;i < 10; i++) {
        printf("pulling: %d\n", *(int*)queue_pull(q));
    }
    return NULL;
}

int main(int argc, char **argv) {
    // if (argc != 3) {
    //     printf("usage: %s test_number return_code\n", argv[0]);
    //     exit(1);
    // }
    q = queue_create(10);

    pthread_t pt;

    for (int i = 0; i < 3; i++) {
        pthread_create(&pt, NULL, generator, NULL);
        
        
    }
    

    pthread_t pt_2;
    pthread_create(&pt_2, NULL, consumer, NULL);

    pthread_join(pt, NULL);
    pthread_join(pt_2, NULL);

    queue_destroy(q);
    return 0;
}
