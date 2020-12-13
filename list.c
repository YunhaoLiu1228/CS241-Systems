#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct node {
    int data;
    struct node *next;
} node;

typedef struct list {
    node *head;
    pthread_mutex_t mutex;
    size_t size;
} list;

static list* test_list;

list* create_list() {
    list* l = (list*) malloc(sizeof(list));
    l->head = NULL;
    pthread_mutex_init(&(l->mutex), NULL);
    l->size = 0;

    return l;
}

void destroy_list(list* l) {

    pthread_mutex_lock(&(l->mutex));
    node* n = l->head;
    node* temp;

    while (n) {
      temp = n->next;
      free(n);
      n = temp;
    }
    pthread_mutex_unlock(&(l->mutex));
    pthread_mutex_destroy(&(l->mutex));
    free(l);
    l = NULL;

}

// display the list
void print_list(list* l) {
    node* n = l->head;
    printf("\n");
        
    //start from the beginning
    while(n != NULL) {
        printf("[%d] -> ",n->data);
        n = n->next;
    }
    
    printf("NULL\n");

}

// inserts at head
void insert(list* l, int data) {
    
    pthread_mutex_lock(&(l->mutex));
    node *n = (node*) malloc(sizeof(node));
        
    n->data = data;
    n->next = l->head;
    l->head = n;
    l->size++;
    pthread_mutex_unlock(&(l->mutex));
}

//is list empty
bool isEmpty(list* l) {
    return l->size == 0;
}


node* delete(list* l, int data) {

    pthread_mutex_lock(&(l->mutex));
    node* curr = l->head;
    node* prev = NULL;
        
    if (!l->head) {
        return NULL;
    }

    while (curr->data != data) {
        if(!curr->next) {
            return NULL;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }

    if (curr != l->head) {
        prev->next = curr->next;
        
    } else {
        l->head = l->head->next;
    }    
    
    l->size--;
    pthread_mutex_unlock(&(l->mutex));
    return curr;
}

void* thread_func(void* arg) {
    insert(test_list, (int)arg);
    return NULL;
}

void list_test() {
    test_list = create_list();
    printf("empty list:");
    print_list(test_list);

    pthread_t pids[5];
    for (int i = 0; i < 5; i++) {
        pthread_create(&(pids[i]), NULL, thread_func, (void*)i);
    }
    
    for (int i = 0; i < 5; i++) {
        pthread_join(pids[i], NULL);
    }

    printf("populated list:");
    print_list(test_list);

    destroy_list(test_list);
    return;
}

int main(int argc, char** argv) {
    list_test();
    return 0;
}