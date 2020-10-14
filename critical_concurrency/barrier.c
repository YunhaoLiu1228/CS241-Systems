/**
 * critical_concurrency
 * CS 241 - Fall 2020
 */
#include "barrier.h"
#include <stdio.h>

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    if (!barrier) return 1;

    pthread_mutex_destroy(&(barrier->mtx)); 
    pthread_cond_destroy(&(barrier->cv));
    
    return 0;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {

    if (!barrier ) return 1;

    barrier->n_threads = num_threads;
    barrier->times_used = 1;
    barrier->count = 0;

    pthread_cond_init(&(barrier->cv), NULL);
    pthread_mutex_init(&(barrier->mtx), NULL);
    
    return 0;
}

int barrier_wait(barrier_t *barrier) {
    // [pop it] lock it polka dot it
    pthread_mutex_lock(&(barrier->mtx));

    while (barrier->times_used == 0) {
        pthread_cond_wait(&(barrier->cv), &(barrier->mtx));
    }

    barrier->count++;
    
    if (barrier->count == barrier->n_threads) {
        barrier->times_used = 0;
        barrier->count--;
        
    } else {
        while (barrier->times_used == 1 && barrier->count != barrier->n_threads) {
            pthread_cond_wait(&(barrier->cv), &(barrier->mtx));
        }

        barrier->count--;

        if (barrier->count == 0) barrier->times_used = 1;

    }

    pthread_cond_broadcast(&(barrier->cv));
    pthread_mutex_unlock(&barrier->mtx);
    return 0;
}
