/**
 * teaching_threads
 * CS 241 - Fall 2020
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct task_t {
    int* data;
    size_t num_items;
} task_t;
 
/* You should create a start routine for your threads. */
/**
 * multi-threaded solution to reduce().
 *
 * This method takes in a `list` of ints and returns a int that is a "reduced"
 * version of the list, but does so with `num_threads` threads.
 *
 * Note: that this function DOES NOT modify the original list.
 *
 * `list`- is a pointer to the begining of an array of ints.
 * `length` - is how many ints are in the array of ints.
 * `reducer_func` - is the reducer used to transform a list of ints to an int.
 * `base_case` - is the base case that the reducer will use.
 * `num_threads` - is how many threads (in addition to the main thread) are used
 * in the parallelization.
 */
int par_reduce(int* list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {

    int result = base_case;

    // the number of elements of the array that each p_thread will handle
    size_t frac = list_len/num_threads;

    // make the p_threads
    pthread_t tids[num_threads]; 

    for (size_t i = 0; i < num_threads; i++) {
        task_t* t = malloc(sizeof(task_t));
        if (t == NULL) {
            printf("Uh oh\n");
            exit(1);
        }
        t->data = list;
        t->num_items = frac;

        pthread_create(&tids[i], NULL, (void*)reduce_func, (void *)t);
    }

    // join the threads
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(tids[i], NULL);
    }
    printf("result: %d\n", result);
    return result;
}
