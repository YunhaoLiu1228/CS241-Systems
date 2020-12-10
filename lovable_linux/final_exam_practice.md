# Angrave's 2020 Acme CS 241 Exam Prep		
## A.K.A. Preparing for the Final Exam & Beyond CS 241... 

Some of the questions require research (wikibook; websearch; wikipedia). 
It is accepted to work together and discuss answers, but please make an honest attempt first! 
Be ready to discuss and clear confusions & misconceptions in your last discussion section.
The final will also include pthreads, fork-exec-wait questions and virtual memory address translation. 
Be awesome. Angrave.

## 1. C 


1.	What are the differences between a library call and a system call? Include an example of each.
    System calls are executed by the system in the system kernel and are not portable. They change the execution mode of the program from user mode to kernel mode. An example of a system call is sbrk().
    Library calls are those that are part of the C standard library. An example of a library call is strcmp()

2.	What is the `*` operator in C? What is the `&` operator? Give an example of each.
    The `*` operator dereferences a pointer, aka get the data that the pointer was pointing to. The `&` operator takes the address of data and returns the pointer to it.

3.	When is `strlen(s)` != `1+strlen(s+1)` ?
    When s is the empty string


4.	How are C strings represented in memory? What is the wrong with `malloc(strlen(s))` when copying strings?
    C strings are represented as `char*`s. `malloc(strlen(s))` is incorrect because to allocate memory for a C string, you should do `malloc(sizeof(char))` to get the appropriate amount of memory.

5.	Implement a truncation function `void trunc(char*s,size_t max)` to ensure strings are not too long with the following edge cases.
```
if (length < max)
    strcmp(trunc(s, max), s) == 0
else if (s is NULL)
    trunc(s, max) == NULL
else
    strlen(trunc(s, max)) <= max
    // i.e. char s[]="abcdefgh; trunc(s,3); s == "abc". 
```


6.	Complete the following function to create a deep-copy on the heap of the argv array. Set the result pointer to point to your array. The only library calls you may use are malloc and memcpy. You may not use strdup.

    `void duplicate(char **argv, char ***result);` 

7.	Write a program that reads a series of lines from `stdin` and prints them to `stdout` using `fgets` or `getline`. Your program should stop if a read error or end of file occurs. The last text line may not have a newline char.

## 2. Memory 

1.	Explain how a virtual address is converted into a physical address using a multi-level page table. You may use a concrete example e.g. a 64bit machine with 4KB pages. 
Assume a 32bit machine; there are then 2^32 possible addresses. A typical linux page size is 2^12 addresses. Then there are (2^32)addresses/(2^12)addresses per page  = 2^20 page frames. So then 20 bits are needed to number all the possible frames. In a 32 bit address, the first 20 bits will be used to access the physical address. The first 10 bits are used to get the page table entry, and then the next 10 bits are used to get the physical frame/memory page. The last 12 bits, the offset, are used to index and find the actual data asked for by the user.

2.	Explain Knuth's and the Buddy allocation scheme. Discuss internal & external Fragmentation.

3.	What is the difference between the MMU and TLB? What is the purpose of each?
The MMU is a hardware unit; it translates virtual addresses to physical addresses. The TLB is an associative cache of page table entries used by the MMU to speed up virtual address translation since it allows the necessity of accessing main memory every time a virtual address is mapped to be avoided.

4.	Assuming 4KB page tables what is the page number and offset for virtual address 0x12345678  ?

5.	What is a page fault? When is it an error? When is it not an error?
A page fault occurs when a process acceses an address in a frame missing in memory. It is an error when the page fault is major (mapping to the page is exclusively on disk) or invalid (address was non-readable or non-writable - segfault). It is not an error when the page fault is minor (no mapping for the page but the address is valid - the OS will just make the page and continue)

6.	What is Spatial and Temporal Locality? Swapping? Swap file? Demand Paging?

## 3. Processes and Threads 

1.	What resources are shared between threads in the same process?
    Resources shared are the heap, the global variables, and the program code (text segment). Also included is signals, the cwd, and open file descriptors. (NOT the stack)

2.	Explain the operating system actions required to perform a process context switch

3.	Explain the actions required to perform a thread context switch to a thread in the same process

4.	How can a process be orphaned? What does the process do about it?
    A process is orphaned when the parent process dies without waiting on its children. When this happens the orphaned process will be assigned to init, aka the first process. When the orphans finish they will briefly become zombies before being removed by init, which automatically waits on all its children.

5.	How do you create a process zombie?
    A process zombie occurs when the parent process doesn't wait on its children and the child then terminates.

6.	Under what conditions will a multi-threaded process exit? (List at least 4)

## 4. Scheduling 
1.	Define arrival time, pre-emption, turnaround time, waiting time and response time in the context of scheduling algorithms. What is starvation?  Which scheduling policies have the possibility of resulting in starvation?
    - Arrival time: the time at which a process first arrives at the ready queue, and is ready to start executing
    - Pre-emption: allows the existing processes to be removed immediately if a more preferred process is added to the ready queue (may help with starvation)
    - Turnaround time: is the total time from when the process arrives to when it ends (end_time - arrival_time)
    - Waiting time: the total wait time or the total time that a process is on the ready queue (end_time - arrival_time - run_time)
    - Response time: is the total latency (time) that it takes from when the process arrives to when the CPU actually starts working on it. start_time - arrival_time
    - Starvation: when a process is perpetually denied necessary resources to process its work. Can occur due to the convoy effect - when a process takes up a lot of the CPU time, leaving all other processes with potentially smaller resource needs following like a Convoy Behind them. Some scheduling policies that result in starvation are SJF and highest priority.

2.	Which scheduling algorithm results the smallest average wait time?
    SJF

3.	What scheduling algorithm has the longest average response time? shortest total wait time?

4.	Describe Round-Robin scheduling and its performance advantages and disadvantages.

5.	Describe the First Come First Serve (FCFS) scheduling algorithm. Explain how it leads to the convoy effect.
    In FCFS processes are scheduled to execute in the order of arrival. This may lead to the convoy effect if one of the earlier processes takes up a lot of CPU time, leaving the potentially shorter processes that arrived later to be starved of CPU time.

6.	Describe the Pre-emptive and Non-preemptive SJF scheduling algorithms. 

7.	How does the length of the time quantum affect Round-Robin scheduling? What is the problem if the quantum is too small? In the limit of large time slices Round Robin is identical to _____?

8.	What reasons might cause a scheduler switch a process from the running to the ready state?

## 5. Synchronization and Deadlock

1.	Define circular wait, mutual exclusion, hold and wait, and no-preemption. How are these related to deadlock?
    1) Mutual exclusion: At least one resource is held in a non-shareable mode (only one process can use the resource at any given instant of time)
    2) Hold and wait: a process is currently holding at least one resource and requesting additional resources which are being held by other processes.
    3) No preemption: a resource can be released only voluntarily by the process holding it.
    4) Circular wait: each process must be waiting for a resource which is being held by another process, which in turn is waiting for the first process to release the resource
    These four conditions are known as the Coffman conditions and only when all four are simultaneouly met a deadlock can arise

2.	What problem does the Banker's Algorithm solve?
    The Banker's Algorithm solves the problem of deadlock prevention

3.	What is the difference between Deadlock Prevention, Deadlock Detection and Deadlock Avoidance?

4.	Sketch how to use condition-variable based barrier to ensure your main game loop does not start until the audio and graphic threads have initialized the hardware and are ready.

5.	Implement a producer-consumer fixed sized array using condition variables and mutex lock.

6.	Create an incorrect solution to the CSP for 2 processes that breaks: i) Mutual exclusion. ii) Bounded wait.

7.	Create a reader-writer implementation that suffers from a subtle problem. Explain your subtle bug.

## 6. IPC and signals

1.	Write brief code to redirect future standard output to a file.
    In bash: ./command > file
    In C:
        #include <stdio.h>
        #include <unistd.h>
        #include <stdlib.h>
        #include <stdio.h>

        int main(int argc, char** argv) {
            int out = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            dup2(out, 1);
            fprintf(stdout, "this is stdout\n");
            return 0;
        }


2.	Write a brief code example that uses dup2 and fork to redirect a child process output to a pipe

3.	Give an example of kernel generated signal. List 2 calls that can a process can use to generate a SIGUSR1.

4.	What signals can be caught or ignored?
    SIGINT, SIGTERM, and SIGQUIT are some signals that can be caught or ignored

5.	What signals cannot be caught? What is signal disposition?
    SIGKILL (TERM) and SIGSTOP (STOP) cannot be caught and handled.

6.	Write code that uses sigaction and a signal set to create a SIGALRM handler.

7.	Why is it unsafe to call printf, and malloc inside a signal handler?

## 7. Networking 

1.	Explain the purpose of `socket`, `bind`, `listen`, and `accept` functions
    (from man pages)
    socket: creates an endpoint for communication and returns a file descriptor that refers to that endpoint
    bind: assigns the address specified by addr to the socket referred to by the file descriptor sockfd parameter.
    listen: marks the socket referred to by sockfd as a passive socket, that is, as a socket that will be used to accept incoming connection requests using accept
    accept: accepts a connection on a socket

2.	Write brief (single-threaded) code using `getaddrinfo` to create a UDP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

3.	Write brief (single-threaded) code using `getaddrinfo` to create a TCP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

4.	Explain the main differences between using `select` and `epoll`. What are edge- and level-triggered epoll modes?

5.	Describe the services provided by TCP but not UDP. 
    TCP provides assurance of message delivery, which UDP does not. There are also many features of TCP such as ports with IP, retransmission packets, out of order packets, duplicate packets, error correction, flow control, congestion control, and connection-orientation / life cycle orientation.

6.	How does TCP connection establishment work? And how does it affect latency in HTTP1.0 vs HTTP1.1?

7.	Wrap a version of read in a loop to read up to 16KB into a buffer from a pipe or socket. Handle restarts (`EINTR`), and socket-closed events (return 0).

8.	How is Domain Name System (DNS) related to IP and UDP? When does host resolution not cause traffic?

9.	What is NAT and where and why is it used? 

## 8. Files 

1.	Write code that uses `fseek`, `ftell`, `read` and `write` to copy the second half of the contents of a file to a `pipe`.

2.	Write code that uses `open`, `fstat`, `mmap` to print in reverse the contents of a file to `stderr`.

3.	Write brief code to create a symbolic link and hard link to the file /etc/password

4.	"Creating a symlink in my home directory to the file /secret.txt succeeds but creating a hard link fails" Why? 

5.	Briefly explain permission bits (including sticky and setuid bits) for files and directories.
    The permissions of a file are represented by a 3-digit octal number, the least significant byte of which corresponds to the read privileges, the middle one to write privileges and the final byte to execute privileges. 
    The setuid bit is an additional bit that files with execute permission may have set. It indicates that when the program is run, it will set the uid of the user to that of the owner of the file.
    The sticky bit today can be set on a directory to allow only the file owner, user owner, or root user to rename or delete the file. 

6.	Write brief code to create a function that returns true (1) only if a path is a directory.
    int is_dir (const char* path) {
        struct stat s;
        if (stat(path, &s)) return 0;
        else return S_ISDIR(s.st_mode);
    }
7.	Write brief code to recursive search user's home directory and sub-directories (use `getenv`) for a file named "xkcd-functional.png' If the file is found, print the full path to stdout.

8.	The file 'installmeplz' can't be run (it's owned by root and is not executable). Explain how to use sudo, chown and chmod shell commands, to change the ownership to you and ensure that it is executable.

## 9. File system 
Assume 10 direct blocks, a pointer to an indirect block, double-indirect, and triple indirect block, and block size 4KB.

1.	A file uses 10 direct blocks, a completely full indirect block and one double-indirect block. The latter has just one entry to a half-full indirect block. How many disk blocks does the file use, including its content, and all indirect, double-indirect blocks, but not the inode itself? A sketch would be useful.

2.	How many i-node reads are required to fetch the file access time at /var/log/dmesg ? Assume the inode of (/) is cached in memory. Would your answer change if the file was created as a symbolic link? Hard link?

3.	What information is stored in an i-node?  What file system information is not? 
    the inode of a file or directory contains metadata about the file as well as pointers to disk blocks so that the file can actually be written to or read from.

4.	Using a version of stat, write code to determine a file's size and return -1 if the file does not exist, return -2 if the file is a directory or -3 if it is a symbolic link.

5.	If an i-node based file uses 10 direct and n single-indirect blocks (1 <= n <= 1024), what is the smallest and largest that the file contents can be in bytes? You can leave your answer as an expression.

6.	When would `fstat(open(path,O_RDONLY),&s)` return different information in s than `lstat(path,&s)`?

## 10. "I know the answer to one exam question because I helped write it"

Create a hard but fair 'spot the lie/mistake' multiple choice or short-answer question. Ideally, 50% can get it correct.
