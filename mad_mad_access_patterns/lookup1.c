/**
 * mad_mad_access_patterns
 * CS 241 - Fall 2020
 */
#include "tree.h"
#include "utils.h"
#include "string.h"

#include <stdlib.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/


int validate_btre(FILE* input) {

  char buff[4];
  fread(buff, 1, 4, input);

  if (strcmp(buff, "BTRE") == 0) {
    return 1;
  }

  return 0;
}

int word_search(FILE* inputfile, char* word, uint32_t offset) {

  if (offset == 0) {  // base case
    printNotFound(word);
    return 0;
  }

  fseek(inputfile, offset, SEEK_SET);

  BinaryTreeNode btnode;
  fread(&btnode, sizeof(BinaryTreeNode), 1, inputfile);
  fseek(inputfile, sizeof(BinaryTreeNode) + offset, SEEK_SET);

  char file_word[30];
  fread(file_word, 30, 1, inputfile);

  if (strcmp(word, file_word) == 0) {
    printFound(file_word, btnode.count, btnode.price);
    return 1;
  }
    
  else if (strcmp(word, file_word) > 0) {
    if (word_search(inputfile, word, btnode.right_child)) {
      return 1;
    }
  }

  else if (strcmp(word, file_word) < 0) {
    if (word_search(inputfile, word, btnode.left_child)) {
      return 1;
    }
  }

  return 0;
}

int main(int argc, char **argv) {

  if (argc < 2) {
    printArgumentUsage();
    exit(1);
  }

  char* in = strdup(argv[1]);
  FILE* inputfile = fopen(in, "r");

  if (!inputfile) {
    openFail(in);
    exit(2);
  }

  if (!validate_btre(inputfile)) {
    formatFail(in);
    exit(2);
  }

  for (int i = 2; i < argc; i++) {
    word_search(inputfile, argv[i], BINTREE_ROOT_NODE_OFFSET);
  }

  fclose(inputfile);
  free(in);
  return 0;
}
