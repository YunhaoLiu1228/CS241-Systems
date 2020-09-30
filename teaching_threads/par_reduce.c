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
    int* list_;
    size_t list_len_;
    reducer reduce_func_;
    int base_case_;
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

    pthread_t id;
    int* r;

    task_t* task = malloc(sizeof(task_t));
    task->list_ = list;
    task->list_len_ = list_len;
    task->reduce_func_ = reduce_func;
    task->base_case_ = base_case;


    for (size_t i = 0; i < num_threads; ++i) {
        pthread_create(&id, NULL, my_func, task);
    }

    pthread_join(id, (void**)&r);

    return *r;
}
