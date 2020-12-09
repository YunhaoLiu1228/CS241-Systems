/**
 * malloc
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>


typedef struct meta_data { 
    void *ptr;
    size_t size;
    bool free;
    struct meta_data* next;
    struct meta_data* prev;
} meta_data;

static meta_data * head = NULL;
static size_t free_mem;

int force_printf(const char *format, ...) {
    static char buf[4096]; //fine because malloc is not multithreaded

    va_list args;
    va_start(args, format);
    vsnprintf(buf, 4096, format, args);
    buf[4095] = '\0'; // to be safe
    va_end(args);
    write(1, buf, strlen(buf));

    return 0;
}

// inserts an itemn to the list AT HEAD (first!!!)
// add to list if the memory is AVALIABLE (free = true)
void list_add(meta_data* block) {
    if (!block) return;

    block->free = true;
    
    if (head == NULL) {
        head = block;
        head->next = NULL;
        head->prev = NULL;
    } else {
        block->next = head;
        head->prev = block;
        head = block;
    }
    
}

void list_remove(meta_data* block) {
    if (!block) return;
    
    block->free = false;

    if (block == head) {
        meta_data* temp = block->next;
        if (temp) {
            temp->prev = NULL;
        }
        head = temp;

    } else {
        if (block->next) {
            block->next->prev = block->prev;
        }
        // not at head so prev is true
        block->prev->next = block->next;
    }
}

void merge_blocks(meta_data* block) {
    block->size += block->prev->size + sizeof(meta_data);
    block->prev = block->prev->prev;
    
    if (!block->prev) {
        head = block;
        
    } else {
        block->prev->next = block;
    }
}

void split_block(meta_data* block, size_t split_size) {

    size_t rem_size = block->size - split_size - sizeof(meta_data);
    // if (split_size < 1024 || rem_size < 1024) return;

    meta_data* new_block = (void*)block + sizeof(meta_data) + split_size;
    new_block->ptr = new_block + 1;
    new_block->size = rem_size;
    new_block->free = true;

    free_mem += new_block->size;

    block->size = split_size;
    
    new_block->next = block;
    new_block->prev = block->prev;

    if (!block->prev) {
        head = new_block;
        
    } else {
        block->prev->next = new_block;    
    }

    block->prev = new_block;
    if (new_block->prev && new_block->prev->free) merge_blocks(new_block);


    // meta_data* new_block = (void*)block + sizeof(meta_data) + split_size;

    // new_block->ptr = (void*)new_block + sizeof(meta_data);

    // list_add(new_block);

    // block->size = split_size;
    // block->free = false;


}
/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
//g
void *calloc(size_t num, size_t size) {
    void* ptr = malloc(num*size);
    
    if (!ptr) return NULL;
    
    memset(ptr, 0, num*size);
    
    return ptr;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {

    /* See if we have free space of enough size. */ 
    meta_data* p = head;
    meta_data* chosen = NULL;

    while (free_mem >= size && p != NULL) {
        if (p->free && p->size >= size) {
            if (p->size >= size && p->free) {
                chosen = p;
                if ((chosen->size >= sizeof(meta_data) + size) && (chosen->size  >= size*2)) {
                    split_block(chosen, size);  
                }

                break;
            }
        }

        p = p->next; 
    }
    // block found :D
    if (chosen) {
        chosen->free = false;
        return chosen->ptr; 
    }
    
    chosen = sbrk(sizeof(meta_data));
    chosen->ptr = sbrk(0);

    if (sbrk(size) == (void*)-1) {
        return NULL; 
    }
    chosen->ptr = (void*)chosen + sizeof(meta_data);
    chosen->next = head;
    chosen->prev = NULL;
    chosen->size = size;
    chosen->free = false;

    if (head != NULL) {
        head->prev = chosen;
    }

    head = chosen;

    return chosen->ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    if (ptr == NULL) return; 

    meta_data* p = (meta_data*)ptr - 1;
    if (p->free) return;
    else p->free = true;

    free_mem += (p->size + sizeof(meta_data));

    if (p->prev) {
        if (p->prev->free)  merge_blocks(p);
    }
    if (p->next) {
        if (p->next->free) merge_blocks(p->next);
    }
    //merge_blocks();
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    if (!ptr) return malloc(size);

    meta_data* entry = ((meta_data*)ptr) - 1;

    if (size == entry->size) {
        return ptr;
    }

    void* newptr;
    if (size < entry->size) {
        if (entry->size < sizeof(meta_data) + size) {
            newptr = malloc(size);
            memmove(newptr, ptr, size);

        } else {
            split_block(entry, size);
            return entry->ptr;
        }
    }

    else {  // size > temp->size
        if (entry->prev && entry->prev->free && entry->size + entry->prev->size >= size) {
            merge_blocks(entry);
            split_block(entry,size);
            return entry->ptr;
        } else {
            newptr = malloc(size);
            memmove(newptr, entry->ptr, entry->size);

        }
    }
    free(ptr);
    return newptr;
    // void* newptr = malloc(size);

    // if (newptr == (void*)-1) return NULL;

    // size_t new_size = entry->size < size ? entry->size : size;
    // memcpy(newptr, ptr, new_size);
    // free(ptr);
    // return newptr;

    // if (entry->free)return ptr;
    // if (size < entry_size*2 || size-entry_size < 1024) return ptr;

    // void* newptr = malloc(size);

    // if (newptr == (void*)-1) return NULL;

    // size_t min_size = entry_size < size ? entry_size : size;
    // memmove(newptr, ptr, min_size);
    // free(ptr);
    // return newptr;
}
