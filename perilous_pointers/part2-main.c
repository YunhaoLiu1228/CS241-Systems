/**
 * perilous_pointers
 * CS 241 - Fall 2020
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {

    // 1: 
    first_step(81);

    // 2:
    int a = 132;
    second_step(&a); 

    // 3:
   int** b = malloc(sizeof(int*) *10);
   int i;
   for (i = 0; i < 10; i++) {
       b[i] = malloc(sizeof(int));
       *(b[i])= 8942;
   }
    
    double_step(b);

    for (i = 0; i < 10; i++) {
       free(*b);
       *b= NULL;
   }
    //TODO: free dis
   // free(b);
   // b = NULL;

    // 4:
    char* c = malloc(sizeof(char));
    *c = 'a';
    int* d = (int*)(c+5);
    *d = 15;
    //printf("%d\n", *d);
    strange_step(c);

    free(c);
    c = NULL;

    // 5:
    i = 0;
    void* e = &i;
    empty_step(e);

    // 6:
    void* f;
    char* g = "uwuu";
    f = g;
    two_step(f, g);

    // 7:
    char* x = "asdf";
    char* y= x;
    y +=2 ;
    char* z = y;
    z +=2;
    three_step(x, y, z);

    // 8:
    // third[3] == second[2] + 8 && second[2] == first[1] + 8
    x = "abcd";
    y = "abjd";
    z = "abrr";
    //char cha = *y+2;
    //printf("%c", cha);
    step_step_step(x,y,z);

    // 9:
    x = "66";
    int num = 54;
    it_may_be_odd(x, num);

    // 10:
    char str[] = "CS241,CS241,CS241";
    tok_step(str);

    // 11:   
    char chars[4];
    chars[0] = 1;
    chars[1] = 3;
    chars[2] = 9;
    chars[3] = 2;

    void* blue = chars;
    void* oran = chars;

    the_end(blue, oran);
    

}
