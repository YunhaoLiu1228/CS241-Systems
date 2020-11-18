/**
 * mad_mad_access_patterns
 * CS 241 - Fall 2020
 */
#include "tree.h"
#include "utils.h"
#include "string.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/
int word_search(char* addr, char* word, uint32_t offset) {
  if (offset == 0) {  // base case
    printNotFound(word);
    return 0;
  }

  BinaryTreeNode* btnode = (BinaryTreeNode *) (addr + offset);

  if (strcmp(word, btnode->word) == 0) {
    printFound(btnode->word, btnode->count, btnode->price);
    return 1;
  }
    
  else if (strcmp(word, btnode->word) > 0) {
      if (word_search(addr, word, btnode->right_child)) {
        return 1;
      }
  }

  else if (strcmp(word, btnode->word) < 0) {
    if (word_search(addr, word, btnode->left_child)) {
      return 1;
    }
  }

  return 0;
}

int validate_btre(char* str) {

  if (strncmp(str, "BTRE", 4) == 0) {
    return 1;
  }

  return 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printArgumentUsage();
    exit(1);
  }

  char* in = strdup(argv[1]);

  struct stat sb;
  int fd = open(in, O_RDONLY);

  if (fd == -1) {
    openFail(in);
    exit(2);
  }

  if (fstat(fd, &sb) == -1) {
    openFail(in);
    exit(2);
  }

  void* addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    mmapFail(in);
    exit(2);
  }


  if (!validate_btre(addr)) {
    formatFail(in);
    exit(2);
  }

  for (int i = 2; i < argc; i++) {
    word_search(addr, argv[i], BINTREE_ROOT_NODE_OFFSET);
  }

  close(fd);
  free(in);
  return 0;
}
