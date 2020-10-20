/**
 * password_cracker
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crypt.h>
#include <stdbool.h>

#include "includes/queue.h"
#include "cracker1.h"
#include "format.h"
#include "utils.h"

queue* task_queue = NULL;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

int recovered_passwords = 0;
int failed_passwords = 0;

typedef struct my_task {
    char* username;
    char* pass_hashed;
    char* pass_clue;

} my_task;

my_task* create_task(char* user, char* hash, char* clue) {
    my_task* t = malloc(sizeof(my_task));

    t->username = strdup(user);
    t->pass_hashed = strdup(hash);
    t->pass_clue = strdup(clue);

    return t;
}

void destroy_task(my_task* this) {
    free(this->username);
    free(this->pass_clue);
    free(this->pass_hashed);
    free(this);
}

void* cracker(void* arg) {

    struct crypt_data cd;
    cd.initialized = 0;
    int thread_id = (long) arg;
    my_task *task = NULL;

    while ((task = queue_pull(task_queue))) {
        v1_print_thread_start(thread_id, task->username);

        double start_time = getThreadCPUTime();
        int num_hashes = 0;
        bool success = false;

        char* password = strdup(task->pass_clue);   //TODO: FREE THIS
        int prefix_len = getPrefixLength(password);   // number of chars in the clue (i.e. not .s )


        setStringPosition(password + prefix_len, 0);  // sets chars after password clue to 'a'
                                                    // if password was "qyohph..", after set string pos password = "qyohphaa"
        //printf("PASS: %s\n", password);

        while (true) {
            double elapsed = getThreadCPUTime() - start_time;
            num_hashes++;
            char* guess_hash = crypt_r(password, "xx", &cd);
            
            if (strcmp(task->pass_hashed, guess_hash) == 0) {       // strcmp = 0 when equal !!!

                pthread_mutex_lock(&m);
                    v1_print_thread_result(thread_id, task->username, password, num_hashes, elapsed, 0);
                    success = true;
                    recovered_passwords++;
                pthread_mutex_unlock(&m);

                break;
            }

            // increment the letters starting at the end of the string one by one
            incrementString(password);
            // compare to the prefix
            if (strncmp(password, task->pass_clue, prefix_len)) break;
        }

        if (!success) {
            double elapsed_fail = getThreadCPUTime() - start_time;

            pthread_mutex_lock(&m);
                v1_print_thread_result(thread_id, task->username, NULL, num_hashes, elapsed_fail, 1);
                failed_passwords++;
            pthread_mutex_unlock(&m);
        }

        free(password);
        destroy_task(task);
  }
  queue_push(task_queue, NULL);
  return NULL;
}

int start(size_t thread_count) {
    
    char* line = NULL;
    size_t size = 0;
    ssize_t read;

    task_queue = queue_create(0);
    
    read = getline(&line,&size, stdin);
    while (read != -1) {
    
        if (line[read-1] == '\n' && read != 0) line[read-1] = '\0';
        
        char* username = strtok(line, " ");
        char* hash = strtok(NULL, " ");
        char* clue = strtok(NULL, " ");
        
        my_task *task = create_task(username, hash, clue);
        queue_push(task_queue, task);
        read = getline(&line,&size, stdin);
    }

    queue_push(task_queue, NULL);

    pthread_t tids[thread_count];

    for (size_t i = 0; i < thread_count; i++) {
        pthread_create(tids+i, NULL, cracker, (void *)(i+1));
    }
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(tids[i], NULL);
    }
    
    v1_print_summary(recovered_passwords, failed_passwords);

    // cleanup
    free(line);
    queue_destroy(task_queue);
    pthread_mutex_destroy(&m);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}