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
   int** b = (int**)malloc(sizeof(int*) * 10);
   int i;
   for (i = 0; i < 10; i++) {
       b[i] = (int*)malloc(sizeof(int)*10);
       *(b[i])= 8942;
   }
    
    double_step(b);
    //TODO: free dis

    // 4:
    char* c = malloc(sizeof(char));
    *c = 'a';
    int* d = (int*)(c+5);
    *d = 15;
    //printf("%d\n", *d);
    strange_step(c);

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
    int* blu = malloc(sizeof(char*));
    *blu = 1;
    void* oran = malloc(sizeof(int*));
    oran = "123";

    //blu = oran;
    //printf("%d\n", (int)sizeof(*blu));

    //int ptr = 12;
    //*blu = "hi";
    //o = blu;

    the_end(blu, oran);

}
