/**
 * critical_concurrency
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "semamore.h"

static Semamore* mutex; 
  
void* thread(void* arg) 
{ 
    //wait 
    semm_wait(mutex); 
    printf("\nEntered..\n"); 
  
    //critical section 
    sleep(4); 
      
    //signal 
    printf("\nJust Exiting...\n"); 
    semm_post(mutex); 
    return NULL;
} 

int main(int argc, char **argv) {
    mutex = malloc(sizeof(Semamore));

    semm_init(mutex, 0, 10); 

    pthread_t t1,t2; 
    pthread_create(&t1,NULL,thread,NULL); 
    sleep(2); 
    pthread_create(&t2,NULL,thread,NULL); 

    pthread_join(t1,NULL); 
    pthread_join(t2,NULL);

    semm_destroy(mutex); 
    free(mutex);
    
    return 0; 
}
