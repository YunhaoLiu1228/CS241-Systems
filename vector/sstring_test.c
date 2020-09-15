/**
 * vector
 * CS 241 - Fall 2020
 */
#include "sstring.h"

int main(int argc, char *argv[]) {
    char* input = "this is a char pointer";
    sstring* str = cstr_to_sstring(input);
    
    printf("sstring: ");
    for (size_t i = 0; i < sstr_length(str); i++) {
        printf("%c", **(char**)vector_at(str->string, i));
    }
    printf("\n");

    char* output = sstring_to_cstr(str);
    printf("c_string: '%s' \n", output);

    // -------------- test append ---------------- //
    char* input2 = " hiyaaa";
    sstring* str_to_append = cstr_to_sstring(input2);
    printf("%d \n", sstring_append(str, str_to_append));
     for (size_t i = 0; i < sstr_length(str); i++) {
        printf("%c", **(char**)vector_at(str->string, i));
    }
    printf("\n");

    // --------------- test split ---------------- //

    vector* subs = sstring_split(str, ' ');
    for(size_t i = 0; i < vector_size(subs); i++) {
        printf("substr: %s\n", *(char**)vector_at(subs, i));
    }
    return 0;
}
