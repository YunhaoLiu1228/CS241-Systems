/**
 * mini_memcheck
 * CS 241 - Fall 2020
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * GLOBAL VARIABLES
**/

// a pointer to the head of a linked list storing all the metadata 
// corresponding to allocated memory blocks
meta_data* head;

// stores the total number of bytes of memory requested by the user throughout
// the lifetime of the program (excluding metadata)
size_t total_memory_requested = 0;

// stores the total number of bytes of memory freed by the user throughout the lifetime of the program
size_t total_memory_freed = 0;

// stores the number of times the user has tried to realloc or free an invalid pointer
size_t invalid_addresses = 0;



void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {

    if (!filename || !instruction || (long)request_size >= 0) return NULL;

    // so we can use printf
    setvbuf(stdout, NULL, _IONBF, 0);

    // increment memory freed
    total_memory_requested += request_size;

    meta_data* md = malloc(sizeof(meta_data) + request_size);
    printf("md: %p\n", md);
    void* new_mem = md + sizeof(meta_data);
    printf("mem: %p\n", new_mem);

    md->filename = filename;
    md->request_size = request_size;
    md->instruction = instruction; 
    md->next = NULL;    
    
    // do stuff with head IF NULL
    if (!head) {
        head = md;

    // else find the last block in the list
    }  else {
        meta_data* temp = head;

        while(temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = md;
        
    }

    return new_mem;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    return NULL;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    return NULL;
}

void mini_free(void *payload) {
   
}
