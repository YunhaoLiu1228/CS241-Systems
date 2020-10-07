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

void queue_push(queue *this, void *data) {
    if (!this || !data) return;

    if (this->max_size < 0 || this->size <= this->max_size) {

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
    }
}

void *queue_pull(queue *this) {
    if (this->size == 0) {
        printf("Queue is empty!\n");
        return NULL;
    }

    void* last = this->head->data;

    queue_node* temp = this->head->next;

    this->head = temp;
    this->size--;


    return last;
}
