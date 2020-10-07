/**
 * critical_concurrency
 * CS 241 - Fall 2020
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    queue* q = malloc(sizeof(queue));

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->max_size = max_size;
    pthread_mutex_init(&(q->m), NULL);
    pthread_cond_init(&(q->cv), NULL); 

    return q;
}

void queue_destroy(queue *this) {
    if (!this) return;

    queue_node* node = this->head;
    queue_node* temp;
    while (node) {
        temp = node;
        node = node->next;
        free(temp);
        
    }
    free(node);
    pthread_cond_destroy(&(this->cv));
    pthread_mutex_destroy(&(this->m));
    free(this);
}

// needs to block if queue is full (size == max_size)
void queue_push(queue *this, void *data) {

    pthread_mutex_lock(&(this->m));

    while (this->max_size >= 0  && this->size >= this->max_size) {    // only if max_size is capped 
        // block on condition variable this->cv
        pthread_cond_wait(&(this->cv), &(this->m));
    }
    
    if (!this || !data) return;


    queue_node* node = malloc(sizeof(queue_node));
    node->data = data;
    node->next = NULL;
    
    if (!this->head) {
        this->head = node;
        this->tail = node;
    } else {
        this->tail->next = node;
        this->tail = node;
    }

    this->size++;

    // unblock threads blocked on this->cv
    pthread_cond_broadcast(&(this->cv));
    pthread_mutex_unlock(&(this->m));
}

// needs to block if queue is empty (size == 0)
void *queue_pull(queue *this) {
    pthread_mutex_lock(&(this->m));

    while (this->size == 0) {
        // block on condition variable this->cv
        pthread_cond_wait(&(this->cv), &(this->m));
    }

    void* last = this->head->data;

    queue_node* temp = this->head->next;

    this->head = temp;
    this->size--;

    // unblock threads blocked on this->cv
    pthread_cond_broadcast(&(this->cv));
    pthread_mutex_unlock(&(this->m));

    return last;
}
