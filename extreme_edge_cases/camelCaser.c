/**
 * extreme_edge_cases
 * CS 241 - Fall 2020
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Function to remove all spaces from a given string 
void removeSpaces(char *str) 
{ 
    // To keep track of non-space character count 
    int count = 0; 
  
    // Traverse the given string. If current character 
    // is not space, then place it at index 'count++' 
    for (unsigned int i = 0; i < strlen(str); i++) {
        if (str[i] != ' ') {
            str[count++] = str[i]; // here count is incremented 
        }
    }
    str[count] = '\0'; 
} 
//                    0123456789
// example sentence: "this is as"
// str is always all-lowercase!
void camelCase(char* str) {
    if (str == NULL || strlen(str) <= 1) return;

    for (unsigned int i = 0; str[i]; i++) {
        if (str[i] == ' ' && i < strlen(str) - 1) {
            if (i !=0) {
             str[i+1] = toupper(str[i+1]);
            }
        }
    }
}

char* getNonAlpha(char* input) {
    char* chars = malloc(sizeof(char*) * 30);
    char* c = input;
    while(*c != '\0') {
        if (ispunct(*c)) {
            //if (strcmp(chars, "\0") == 0) *chars = *c;
            if (*c == '.' && *c != '!' && *c!= '?') {*c = '\0';}
            else {
                char punct = c[0];
                char* current = &punct;
                strcat(chars, current);
            }
        }
        c++;
    }
    
    return chars;
}

/*
 * param input_str: a C string, which represents an arbitrary number of sentences
 * returns: NULL of input_str is a NULL ptr, else an array of output_s for every 
 *          input_s in the input string, terminated by a NULL pointer.
 *          output is the the concatenation of all words w in input_s after w has been camelCased.
 *          The punctuation from input_s is not added to output_s.
 */

char** camel_caser(const char *input_str) {
    
    if (input_str == NULL) return NULL;

    // say input = "Hello. this is! a basic string to test." ...
    //char* input = strdup(input_str);

    // then output = {"hello", "thisIs", "aBasicStringToTest"}
    char** output = (char**) malloc(sizeof(char*) * 100);       // can there be more than 100 sentences?
    output[100] = (char*) NULL;

    char* str = strdup(input_str);
    char * pch;
    const char* delim = getNonAlpha(str);      // only these constitute the end of sentence
    pch = strtok (str,delim);

    int output_arr_counter = 0;

    while (pch != NULL)
    {
        output[output_arr_counter] = pch;       // put the sentence strings into the output "array" 
        output_arr_counter++;
        pch = strtok (NULL, delim);

        if (pch != NULL && strcmp(pch, "\0") == 0) output[output_arr_counter] = "\0";
    }

    // now all the sentences are split up! 
    // first, put it all to lowercase and strip trailing whitespace
    char **output_cpy = output;

    // for each sentence (char*) in the array
    for (int lc_counter = 0; lc_counter < output_arr_counter; lc_counter++){
        char** location = output_cpy + lc_counter;
        char* curr_sentence = *location;        // get the current sentence in output

        for (unsigned int char_counter = 0; char_counter < strlen(curr_sentence); char_counter++) {
            char* chptr = curr_sentence + char_counter;
            *chptr = tolower(*chptr);
        }
       //printf("%s\n", curr_sentence);

    }

     // now all the sentences are split up! 
    // first, put it all to lowercase and strip trailing whitespace
    char **output_cpy2 = output;

    // for each sentence (char*) in the array
    for (int lc_counter = 0; lc_counter < output_arr_counter; lc_counter++){
        char** location2 = output_cpy2 + lc_counter;
        char* curr_sentence2 = *location2;        // get the current sentence in output

        for (unsigned int char_counter = 0; char_counter < strlen(curr_sentence2); char_counter++) {
            char* chptr2 = curr_sentence2 + char_counter;
            //char ch = *chptr2;

            // camel case!
            camelCase(chptr2);

            // remove whitespace .. and that's it!
            removeSpaces(chptr2);
        }
       // printf("%s\n", curr_sentence2);

    }


    return output;
}

void destroy(char **result) {
    free(result);
    result = NULL;
    // TODO: Implement me!
    return;
}
