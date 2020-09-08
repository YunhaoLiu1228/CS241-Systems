/**
 * extreme_edge_cases
 * CS 241 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

// return 1 if cases pass, 0 if they fail
int compare_strings(char** input_answers, char** correct_answers) {
    if (input_answers == NULL && correct_answers == NULL ) return 1;
    else if (input_answers == NULL || correct_answers== NULL ) return 0;

    while (*input_answers != NULL && *correct_answers != NULL) {
        if (strcmp(*input_answers, *correct_answers) != 0) return 0;

        input_answers++;
        correct_answers++;
    }

    return 1;
}

/* 
 * params: a function pointer to the camelcaser and destroy functions we need to test (NOT OURS)
 * return: true (1) if all test cases pass, false (0) if any fail
 */

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {

    // TEST 0 : NULL string
    const char* str0 = NULL;
    char** output0 = camelCaser(str0);
    if (!compare_strings(output0, NULL)) return 0;
    //else printf("Passed test 0!\n");

    // TEST 1: basic sentence
     const char* str1 = "Hello.";
     char* answers1[] = {"hello", NULL};
     char** output1 = camelCaser(str1);
     if (!compare_strings(output1, answers1)) return 0;
    // else printf("Passed test 1!\n");
    
    // TEST 2: slightly less basic
     const char* str2 = "Hel&lo.there!";
     char* answers2[] = {"hel" , "lo", "there", NULL};
     char** output2 = camelCaser(str2);
     if (!compare_strings(output2, answers2)) return 0;
    // else printf("Passed test 2!\n");


    // TEST 3: some more sentences
    const char* str3 = "this is! a basic string to test.i am testIG this .";
    char* answers3[] = {"thisIs", "aBasicStringToTest", "iAmTestigThis", NULL};
    char** output3 = camelCaser(str3);
    if (!compare_strings(output3, answers3)) return 0;
  //  else printf("Passed test 3!\n");


    // TEST 4: uppercase me
    const char* str4 = "u p p e r c a s e m e";
    char* answers4[] = {
        "uPPERCASEME",
        NULL
    };
    char** output4 = camelCaser(str4);
    if (!compare_strings(output4, answers4)) return 0;
    //else printf("Passed test 4!\n");

    // TEST 5: numbers
    const char* str5 = "th123Is is4 a seN82930ten9ce.";
    char* answers5[] = {
        "th123isIs4ASen82930ten9ce",
        NULL
    };
    char** output5 = camelCaser(str5);
    if (!compare_strings(output5, answers5)) return 0;
    //else printf("Passed test 5!\n");


    // TEST 6: more punct characters
    const char* str6 = ". her's a sentence!";
    char* answers6[] = {
        "",
        "her",
        "sASentence",
        NULL
    };
    char** output6 = camelCaser(str6);
    if (!compare_strings(output6, answers6)) return 0;
   // else printf("Passed test 6!\n");

    // TEST 7: even MORE punct
    const char* str7 = "this7 *&&  a sente231)nce!";
    char* answers7[] = {
        "this7",
        "",
        "",
        "aSente231",
        "nce",
        NULL
    };
    char** output7 = camelCaser(str7);
    if (!compare_strings(output7, answers7)) return 0; 
  //  else printf("Passed test 7!\n");

    // TEST 8: empty
    const char* str8 = "";
    char* answers8[] = {NULL};
    char** output8 = camelCaser(str8);
    if (!compare_strings(output8, answers8)) return 0; 
    //else printf("Passed test 8!\n");

    // TEST 9: lots of whitespace?
    const char* str9 = "What makes      code good?      Is it camelCased strings? Good      comments? Descriptive variable names, perhaps?";
    char* answers9[] = {
        "whatMakesCodeGood",
        "isItCamelcasedStrings",
        "goodComments",
        "descriptiveVariableNames",
        "perhaps"
        "",
        NULL
};
    char** output9 = camelCaser(str9);
    if (!compare_strings(output9, answers9)) return 0; 
   //else printf("Passed test 9!\n");

    // TEST 10
    const char* str10 = "*&^ ()";
    char* answers10[] = {
        "",
        "",
        "",
        "",
        "",
        NULL
};
    char** output10 = camelCaser(str10);
    if (!compare_strings(output10, answers10)) return 0; 
   // else printf("Passed test 10!\n");

    // TEST 11
    const char* str11 = "an all lowercase string i guess";
    char* answers11[] = {
        NULL
};
    char** output11 = camelCaser(str11);
    if (!compare_strings(output11, answers11)) return 0; 
   // else printf("Passed test 11!\n");

    // TEST 12
    const char* str12 = "     nu12mber4 i.  . l12ke ,,nu99mbers";
    char* answers12[] = {
        "nu12mber4I",
        "",
        "l12ke",
        "",
        NULL
};
    char** output12 = camelCaser(str12);
    if (!compare_strings(output12, answers12)) return 0; 
   // else printf("Passed test 12!\n");


    // TEST 13
    const char* str13 = "lately,\n i've been playing a LOT of solitaire. \nhehe!";
    char* answers13[] = {
        "lately",
        "i",
        "veBeenPlayingALotOfSolitaire",
        "hehe",
        NULL
};
    char** output13 = camelCaser(str13);
    if (!compare_strings(output13, answers13)) return 0; 
    //else printf("Passed test 13!\n");


    // TEST 14
    const char* str14 = ". . . . . . . !@#$%^&*(";
    char* answers14[] = {
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        NULL
};
    char** output14 = camelCaser(str14);
    if (!compare_strings(output14, answers14)) return 0; 
   // else printf("Passed test 14!\n");


    // TEST 15
    const char* str15 = "  .THE fiNAL678 tes t  ! owhw(*&*)(&*^&" ;
    char* answers15[] = {
        "",
        "theFinal678TesT",
        "owhw",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        NULL
};
    char** output15 = camelCaser(str15);
    if (!compare_strings(output15, answers15)) return 0; 
   // else printf("Passed test 15!\n");



    return 1;       // made it to the end! :D
}
