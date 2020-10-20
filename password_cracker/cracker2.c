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
int num_task_handles = 0;
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
    int status;     // 0 = success, 1 = stopped early, 2 = end (no password found), -1 (in progress)
    long start_index;  
    long count; // the number of passwords the thread should try
    size_t tid;
    double time;
    size_t hash_count;
} my_task_handler;

my_task_handler* task_handles;

my_task* create_task(char* user, char* hash, char* clue) {
    my_task* t = malloc(sizeof(my_task));

    t->username = strdup(user);
    t->pass_hashed = strdup(hash);
    t->pass_clue = strdup(clue);

    return t;
}

my_task_handler* create_task_handler(my_task* t, long start_i, long count, size_t tid) {
    my_task_handler* th = malloc(sizeof(my_task_handler));
    
    th->username = strdup(t->username);
    th->pass_clue = strdup(t->pass_clue);
    th->pass_hashed = strdup(t->pass_hashed);
    th->status = -1;
    th->start_index = start_i;
    th->count = count;
    th->tid = tid;
    th->time = 0;
    th->hash_count = 0;
    return th;
}

void destroy_task(my_task* this) {
    free(this->username);
    free(this->pass_clue);
    free(this->pass_hashed);
    free(this);
}

void destroy_task_handle(my_task_handler* this) {
    free(this->username);
    free(this->pass_clue);
    free(this->pass_hashed);
    free(this);
}

void* cracker(void* arg) {
    double start_cracker_time = getCPUTime();
    my_task_handler* task_handle = (my_task_handler*)arg;
    num_task_handles++;
    
    
    int num_hashes = 1;

    pthread_mutex_lock(&m1);
    v2_print_thread_start(task_handle->tid, task_handle->username, task_handle->start_index, task_handle->pass_clue);
    pthread_mutex_unlock(&m1);

    struct crypt_data cd;
    cd.initialized = 0;
    char* hash_guess;

    for (long i = 0; i < task_handle->count; i++) {
        task_handle->hash_count = num_hashes;
        hash_guess = crypt_r(task_handle->pass_clue, "xx", &cd);
        // printf("hasguess: %s\n", hash_guess);
        // printf("pass clue: %s\n", task_handle->pass_clue);
        // printf("pass hash: %s\n", task_handle->pass_hashed);

        if (strcmp(hash_guess, task_handle->pass_hashed) == 0) {        // if a success
            
            double success_time = getThreadCPUTime();
            pthread_mutex_lock(&m2);
            flag = true;
            pthread_mutex_unlock(&m2);

            pthread_mutex_lock(&m1);
            v2_print_thread_result(task_handle->tid, num_hashes, 0);
            pthread_mutex_unlock(&m1);

            task_handle->time = success_time - start_cracker_time;
            task_handle->status = 0;

            return NULL;
        }

        incrementString(task_handle->pass_clue);

        pthread_mutex_lock(&m2);
        if (flag) {
            pthread_mutex_unlock(&m2);

            task_handle->status = 1;
            pthread_mutex_lock(&m1);
            v2_print_thread_result(task_handle->tid, i, 1);
            pthread_mutex_unlock(&m1);
            task_handle->time = getThreadCPUTime() - start_cracker_time;
            return NULL;
        }
        pthread_mutex_unlock(&m2);

        num_hashes++;
    }
    
    task_handle->status = 2;
    pthread_mutex_lock(&m1);
    v2_print_thread_result(task_handle->tid, num_hashes, task_handle->status);
    pthread_mutex_unlock(&m1);

    task_handle->time = getThreadCPUTime() - start_cracker_time;
    return NULL;
}

int start(size_t thread_count) {
    int pass_count = 0;
    // Remember to ONLY crack passwords in other threads
    char* line = NULL;
    size_t size;
    ssize_t read;
    read = getline(&line,&size, stdin);
    task_handles = malloc(sizeof(my_task_handler) * thread_count);
    
    while (read != -1) {
        if (line[read-1] == '\n' && read != 0) line[read-1] = '\0';

        pass_count++;
        //printf("line: %s\n", line);

        char* username = strtok(line, " ");
        char* guess_hash = strtok(NULL, " ");
        char* clue = strtok(NULL, " ");
        my_task* task = create_task(username, guess_hash, clue);

            long start_position;
            long count;

        for (size_t i = 0; i < thread_count ; i++) {    // getSubrange starts at 1 - alternatively pass i + 1 to getSubrange

            char* password = strdup(task->pass_clue);

            int prefix_len = getPrefixLength(task->pass_clue);
    

            getSubrange(strlen(task->pass_clue) - prefix_len, thread_count, i+1, &start_position, &count);
            setStringPosition((password), start_position); // sets chars after password clue to 'a'

            strncpy(password, task->pass_clue, prefix_len);

           // my_task_handler* task_handler = create_task_handler(task, start_position, count, i+1);
           // task_handler->pass_clue = password;
            task_handles[i].username = strdup(task->username);     
            task_handles[i].pass_clue = strdup(password);
            task_handles[i].pass_hashed = strdup(task->pass_hashed);
            task_handles[i].status = -1;
            task_handles[i].start_index = start_position;
            task_handles[i].count = count;
            task_handles[i].tid = i+1;
            task_handles[i].time = 0;
            task_handles[i].hash_count = 0;

            read = getline(&line,&size, stdin);
            free(password);
        }
        
        pthread_t tids[thread_count];

        v2_print_start_user(task->username);

        double start_t = getTime();
        
        for (size_t i = 0; i < thread_count; i++) {
            pthread_create(tids+i, NULL, cracker, (void*)&task_handles[i]); 
        }

        for (size_t i = 0; i < thread_count; i++) {
            pthread_join(tids[i], NULL);
        }
        
        /* read mailboxes */
        int result = 1;
        char *password = NULL;
        int c = 0;
        double cpu_time = 0;

        for (size_t i = 0; i < thread_count; i++) {
            if (task_handles[i].status == 0) {
                result = 0;
                password = task_handles[i].pass_clue;
            }
            c += task_handles[i].hash_count;
            cpu_time += task_handles[i].time;
        }

        pthread_mutex_lock(&m1);
        v2_print_summary(task_handles[0].username, password, c, getTime() - start_t, cpu_time, result);
        pthread_mutex_unlock(&m1);
        free(task->username);
        free(task->pass_hashed);
        free(task->pass_clue);
        free(task);
        
        
    }  
  
    
 //TODO: cleanup stuff
    pthread_mutex_lock(&m1);
    for (int i = 0 ; i < num_task_handles; i++) {
        free(task_handles[i].username);
        free(task_handles[i].pass_clue);
        free(task_handles[i].pass_hashed);

    }
    free(task_handles);

    pthread_mutex_unlock(&m1);

    free(line);
    pthread_mutex_destroy(&m1);
    pthread_mutex_destroy(&m2);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
