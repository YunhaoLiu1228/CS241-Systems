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


//void* insert(meta_data *md, size_t size, const char* filename, size_t line)

/**
 *  | HEAD | ---> | MD | ---> | MD | ---> | MD |
 **/

void* mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    

    if (!filename || !instruction || (long)request_size < 0) return NULL;

    // so we can use printf
   // setvbuf(stdout, NULL, _IONBF, 0);

    // increment memory freed
    total_memory_requested += request_size;


    meta_data* md = (meta_data*)malloc(sizeof(meta_data) + request_size);
  

    md->filename = filename;
    md->request_size = request_size;
    md->instruction = instruction; 
    md->next = NULL;
    
    // do stuff with head IF NULL
    if (!head) {
        head = md;

    // else find the last block in the list
    }  else {
        md->next = head;
        head = md;

    }

    void* new_mem = ++md;
    return new_mem;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {

    if (!filename || !instruction || (long)element_size < 0 || (long)num_elements < 0) return NULL;

    // so we can use printf
   // setvbuf(stdout, NULL, _IONBF, 0);

    // increment memory freed
    total_memory_requested += (num_elements * element_size);
    
    meta_data* md = (meta_data*)calloc(num_elements , sizeof(meta_data) + element_size);

    md->filename = filename;
    md->request_size = num_elements*element_size;
    md->instruction = instruction; 
    md->next = NULL;
    
    // do stuff with head IF NULL
    if (!head) {
        head = md;

    // else find the last block in the list
    }  else {
        md->next = head;
        head = md;

    }

    void* new_mem = ++md;
    return new_mem;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    return NULL;
}

void mini_free(void *payload) {
    //setvbuf(stdout, NULL, _IONBF, 0);
    if (!payload) {
        invalid_addresses++;
        return;
    }

    meta_data* prev = NULL;
    meta_data* next = NULL;

    meta_data* it = head;


    while (it ) {

        next = it->next;
        void* mem = (void *) it + sizeof(meta_data);


        if (mem == payload) {

            if (prev != NULL) {
                prev->next = next;
            } else {
                head = next;
            }

            total_memory_freed += it->request_size;

            free(it);
            return;
        }
        
        prev = it;
        it = next;
    }
    
        invalid_addresses++;


}
