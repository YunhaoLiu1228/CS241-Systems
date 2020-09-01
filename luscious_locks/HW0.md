# Welcome to Homework 0!

For these questions you'll need the mini course and virtual machine (Linux-In-TheBrowser) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away zero?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs241.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Piazza:
https://piazza.com/illinois/fall2020/cs241

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".
		#include <unistd.h>
		#include <stdio.h>

		int main() {
			write(1, "Hi! My name is Emily\n”, 21);
			return 0;
		}
### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```
   #include <unistd.h>
	void write_triangle(int n) {
		int i;
		for( i = 1; i <= n ; i++) {
			int j;
			for (j = 1; j <= i; j++ ) {
				write(STDOUT_FILENO,"*", 1);
			}
			write(STDERR_FILENO,"\n", 1);
		}
	}
### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).

#include <unistd.h>
#include  <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	write(fildes,"Hi! My name is Emily\n",21);
	close(fildes);
	
	return 0;
	
}

### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
	
	int fd;
	char *name = "hello_world.txt";
	fd = open(name, O_WRONLY | O_CREAT, 0644);
	if (fd == -1) {
		perror("Open failed");
		exit(1);
	}
	if (dup2(fd, 1) == -1) {
		perror("dup2 failed");
		exit(1);
	}
	
	char *hi = "Hi! My name is Emily";
	printf("%s\n", hi);
	
	exit(0);
	
}
   - Make sure to print to the file instead of standard out!
5. What are some differences between `write()` and `printf()`?
	- return value: write() returns a ssize_t, printf() returns an int
	- write() writes to a file descriptor, printf() writes to standard output
	- parameters: write() takes three parameters; int fd, const void * buf, and size_t count. printf() takes any number of parameters, the first being the char* format, the remaining (if any) being the directives


## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?
	- at least 8
2. How many bytes are there in a `char`?
	- one byte
3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`
	int: 4
	double: 8
	float: 4
	long: 4
	long long: 8
### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?
	- 0x7fbd9d48

5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?
	- it is equivalent to *(data + 3) or even 3[data]

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
	- it segfaults because the hardware knows that the “hello” char * is part of read-only memory (constant memory) and cannot be written 
7. What does `sizeof("Hello\0World")` return?
	- 14
8. What does `strlen("Hello\0World")` return?
	- 13
9. Give an example of X such that `sizeof(X)` is 3.
	- "hi"
10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.
	char *Y = "hello";
	sizeof(Y); 	// would be 4 on a 32-bit system and 8 on a 64-bit system

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?
	⁃	a for loop iterating through argv starting at 1 and incrementing a counter
	⁃	argc - 1
2. What does `argv[0]` represent?
	-	the program name
### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?
	- they are stored at the top of the process memory layout, above the stack
### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?
	- sizeof(ptr) is 4 because it is the size of the char pointer itself
	- sizeof(array) is 6 because it is the length of the string plus a byte for the zero character at the end to indicate the end of the string

### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?
	- the stack

## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?
	⁃	make it static and put it outside the function
2. What are the differences between heap and stack memory?
	⁃	stack: accesses only local variables, contiguous allocation, allocation/deallocation done by compiler
	⁃	heap: accesses global variables, random order allocation, allocation/deallocation done by programmer
3. Are there other kinds of memory in a process?
	- other than the stack and heap, there is the uninitialized data, the initialized data, and the code text itself
4. Fill in the blank: "In a good C program, for every malloc, there is a free".
### Heap allocation gotchas
5. What is one reason `malloc` can fail?
	⁃	the program has used up all heap memory
6. What are some differences between `time()` and `ctime()`?
	⁃	time() takes a time_t * and returns a time_t
	⁃	ctime() takes a const time_t * and returns a char* and essentially converts the result from time() into a readable form; uses static storage	
7. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```
	- when ptr was first freed, the memory may be used by the heap to account for its freeness. By trying to free it again the heap may be “confused
8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
	- ptr is attempted to be used after it’s freed but what was previously stored in that memory is no longer valid

9. How can one avoid the previous two mistakes? 
	⁃	set the freed pointer to NULL (ptr = NULL) so that it no longer points to invalid memory
### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).
11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.

#include <stdio.h>

struct Person {
	int age;
	struct Person** friends;
	//struct Person* (*friends)[];	// friends <- pointer to array of Person pointers
	char* name;
};

typedef struct Person person;

int main() {
	person* per1 = (person*) malloc(sizeof(person));
	person* per2 = (person*) malloc(sizeof(person));
	
	per1->name = "Agent Smith";
	per1->age = 128;
	
	per2->name = "Sonny Moore";
	per2->age = 256;
	
	per1->friends = &per2;
	per2->friends = &per1;
	
	free(per1);
	free(per2);
	return 0;
}
### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).
13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.

person* create(int age_, char* name_) {
	person* newperson = (person*) malloc(sizeof(person));
	newperson->age = age_;
	newperson->name = strdup(name_);
	newperson->friends = (person**) malloc(10 * sizeof(person*));
	return newperson;
}

void destroy(person* p) {
	free(p->name);
	memset(p, 0, sizeof(person));
	free(p);
}

## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?
	⁃	gets()
	⁃	getchar()
	⁃	scanf()
	⁃	I’m sure there are more that I don’t know about
2. Name one issue with `gets()`.
	⁃	if more bytes are entered than the amount allocated for input then gets() will start writing into memory
### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".

int main() {
	char* str = "Hello 5 World";
	
	char buffer1[10];
	char buffer2[10];
	int num = -1;
	
	sscanf(str, “%9s %d %9s", buffer1, &num, buffer2);
	printf("Result: %s  %d  %s\n", buffer1, num, buffer2);
	
	return 0;
}

### `getline` is useful
4. What does one need to define before including `getline()`?
	⁃	#define _GNU_SOURCE
5. Write a C program to print out the content of a file line-by-line using `getline()`.

#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE

int main() {
	char filename[20];
	printf("%s", "print a file name: \n");
	scanf("%s", filename);
	
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("File was not valid\n");
		exit(EXIT_FAILURE);
	}
	
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	
	while ((read = getline(&line, &len, fp)) != -1) {
		printf("%s", line);
	}
	
	return 0;
}


## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?
	⁃	gcc -g
2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.
3. Are tabs or spaces used to indent the commands after the rule in a Makefile?
	⁃	tabs
4. What does `git commit` do? What's a `sha` in the context of git?
	⁃	it essentially saves the changes to the program to git
	⁃	stands for Simple Hashing Algorithm; a SHA-1 hash is a 40-character hash that is a checksum of the content being stored
5. What does `git log` show you?
	-	it shows the commit logs
6. What does `git status` tell you and how would the contents of `.gitignore` change its output?
	⁃	it “Displays paths that have differences between the index file and the current HEAD commit, paths that have differences between the working tree and the index file, and paths in the working tree that are not tracked by Git (and are not ignored by gitignore[5]). ”
	⁃	Adding a file to .gitignore would change the output of git status since the command would no longer show those files that are now being ignored
7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?
	⁃	the command git push is used to upload local repository content to a remote repository
	⁃	the commit message ‘fixed all bugs’ is not sufficient because it doesn’t give any sort of detailed log of the changes being made to the code.
8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?
	⁃	it means that the subversion branch and the remote git master branch don’t agree on something; i.e. something was pushed/committed to one branch but not the other. The most common way of dealing with this issue is using git pull before pushing.

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Piazza.
- Find, in your opinion, the best and worst C code on the web and post the link to Piazza.
- Write a short C program with a deliberate subtle C bug and post it on Piazza to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on piazza.
