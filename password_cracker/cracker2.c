/**
 * password_cracker
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crypt.h>
#include <stdbool.h>
#include <pthread.h>

#include "includes/queue.h"
#include "cracker1.h"
#include "format.h"
#include "utils.h"

pthread_barrier_t b;

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

int recovered_passwords = 0;
int failed_passwords = 0;

bool flag = false;

//queue* th_queue;


typedef struct my_task {
    char* username;
    char* pass_hashed;
    char* pass_clue;

} my_task;


typedef struct my_task_handler {
    //my_task* task;
    char* username;
    char* pass_hashed;
    char* pass_clue;
    int status;     // 0 = in progress, 1 = found, 2 = end (no password found), -1 = no password found
    long start_index;
    long count;
} my_task_handler;

my_task* create_task(char* user, char* hash, char* clue) {
    my_task* t = malloc(sizeof(my_task));

    t->username = strdup(user);
    t->pass_hashed = strdup(hash);
    t->pass_clue = strdup(clue);

    return t;
}

my_task_handler* create_task_handler(my_task* t, long start_i, long count) {
    my_task_handler* th = malloc(sizeof(my_task_handler));
    
    th->username = strdup(t->username);
    th->pass_clue = strdup(t->pass_clue);
    th->pass_hashed = strdup(t->pass_hashed);
    th->status = 0;
    th->start_index = start_i;
    th->count = count;
    return th;
}

void destroy_task(my_task* this) {
    free(this->username);
    free(this->pass_clue);
    free(this->pass_hashed);
    free(this);
}

void* cracker(void* arg) {
    my_task_handler* task_handle = (my_task_handler*)arg;
    printf("queue: %s\n", task_handle->username);
    return NULL;
}

int start(size_t thread_count) {

    // Remember to ONLY crack passwords in other threads
    char* line = NULL;
    size_t len;
    ssize_t read;
    int c = 0;
    while (( read = getline(&line,&len, stdin)) != -1) {
        printf("C: %d\n", c);
        c++;
        //printf("line: %s\n", line);
        my_task_handler task_handles[thread_count];

        char* username = strtok(line, " ");
        char* guess_hash = strtok(NULL, " ");
        char* clue = strtok(NULL, " ");
        my_task* task = create_task(username, guess_hash, clue);

        for (size_t i = 0; i < thread_count ; i++) {    // getSubrange starts at 1 - alternatively pass i + 1 to getSubrange
            char* password = strdup(task->pass_clue);
            int prefix_len = getPrefixLength(password);

            long start_position;
            long count;
            getSubrange(strlen(task->pass_clue) - prefix_len, thread_count, i+1, &start_position, &count);

            setStringPosition(password + prefix_len, 0); // sets chars after password clue to 'a'

            my_task_handler* task_handler = create_task_handler(task, start_position, count);
            task_handles[i] = *task_handler;
        }
        pthread_t tids[thread_count];

        v2_print_start_user(task->username);

        for (size_t i = 0; i < thread_count; i++) {
            pthread_create(tids+i, NULL, cracker, (void*)&task_handles[i]); 
        }

        for (size_t i = 0; i < thread_count; i++) {
            pthread_join(tids[i], NULL);
        }
        


    }  
  
 


    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
