/**
 * vector
 * CS 241 - Fall 2020
 */
#include "vector.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // test creating a vector
    vector* vec0 = int_vector_create();
    assert(vec0);
    printf("Size: %zu\n", vector_size(vec0));
    printf("Capacity: %zu\n", vector_capacity(vec0));
    printf("Vec is empty: %d\n", vector_empty(vec0));

// -------------- test push_back -------------- //
    size_t arrsize = 10;
    int values[] = {1,5,-103, 32, 1999, 8, -11, 0, 1294, 123};
   

    printf("begin size: %zu\n", vector_size(vec0));
    printf("PUSH_BACK TEST:\n");
    for (size_t iter = 0; iter < arrsize; iter++) {
        printf("pushing back : %d\n", values[iter]);
        vector_push_back(vec0, &values[iter]);
        for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i)));}
        printf("\n");
            printf("Size: %zu\n", vector_size(vec0));

    }
    printf("end size: %zu\n", vector_size(vec0));

    printf("-----------------------------------------------------------------\n");

// -------------- TEST INSERT ------------------ // 
    int insertvalues[] = {-100,-200,-300,-400,-500};
    vector_insert(vec0, 3, &insertvalues[0]);
    for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i)));}
        printf("\n");

    vector_insert(vec0, 1, &insertvalues[1]);
    for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i)));}
        printf("\n");

    vector_insert(vec0, 12, &insertvalues[2]);
    for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i)));}
        printf("\n");

    // try some invalid / wonky stuff
    // vector_insert(vec0, -1, &insertvalues[3]);
    // for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i)));}
    //     printf("\n");
    // vector_insert(vec0, 100, &insertvalues[4]);
    // for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i)));}
    //     printf("\n");


    printf("-----------------------------------------------------------------\n");
// -------------- test getters --------------- //
printf("GET TEST:\n");
for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", *(int*)vector_get(vec0, i));}
        printf("\n");
printf("-----------------------------------------------------------------\n");

// -------------- test setters --------------- //
printf(" size: %zu \n", vector_size(vec0));
printf("SET TEST:\n");
int toset[] = {1,2,3,4,5,6,7,8,9,0,11,12,13};
for (size_t i = 0; i < vector_size(vec0); i++) {vector_set(vec0, i, &toset[i]);}
for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", *(int*)vector_get(vec0, i));}
        printf("\n");
printf("-----------------------------------------------------------------\n");
// ------------- test front and back ---------------  //

printf("Vector front: %d\n", **(int**)vector_front(vec0));
printf("Vector back: %d\n", **(int**)vector_back(vec0));

printf("-----------------------------------------------------------------\n");

// -------------- test erase ---------------- //
printf(" size: %zu\n", vector_size(vec0));
    printf("Erasing: vector[1] ...\n");
    vector_erase(vec0, 1);
    for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i))); }
    printf("\n");

printf("-----------------------------------------------------------------\n");
// ------------- test pop_back ---------------  //
    printf("begin size: %zu\n", vector_size(vec0));
    printf("POP_BACK TEST:\n");
   for (int iter = vector_size(vec0)-1; iter>=0; iter--) {
        printf("popping back : %d\n", **(int**)vector_at(vec0, iter));
        vector_pop_back(vec0);
        for (size_t i = 0; i < vector_size(vec0); i++) {printf(" %d ", **((int**)vector_at(vec0, i)));}
        printf("\n");
    }
    printf("end size: %zu\n", vector_size(vec0));
    
    
// --------------------------------------------- //
    return 0;
}
