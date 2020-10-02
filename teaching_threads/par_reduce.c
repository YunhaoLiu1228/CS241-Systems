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

#define MIN( a, b ) ( ( a < b) ? a : b ) 

/* You might need a struct for each task ... */
typedef struct task_t {
    int* list_;
    size_t list_len_;
    reducer reduce_func_;
    int base_case_;
    int frac_;
} task_t;


void* my_func(void* argv) {
    int* retval = malloc(sizeof(int));
    task_t* t = argv;
    
    *retval = reduce(t->list_, t->list_len_, t->reduce_func_, t->base_case_);

    pthread_exit( retval);

    return NULL;
}
 

int par_reduce(int* list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {

    //pthread_t id[MIN(num_threads, list_len)];
    int min = MIN(num_threads, list_len);

    int frac = (list_len + num_threads - 1) / num_threads;

    pthread_t id[min];
    
    int* r;

    task_t* task = malloc(sizeof(task_t));
    task->list_ = list;
    task->list_len_ = list_len;
    task->reduce_func_ = reduce_func;
    task->base_case_ = base_case;
    task->frac_ = frac;



    for (int i = 0; i < min; i++) {
        pthread_create(&id[i], NULL, my_func, task);
    }

    for (int i = 0; i < min; i++) {
        pthread_join(id[i], (void**)&r);
    }
    

    return *r;
}
