/**
 * deepfried_dd
 * CS 241 - Fall 2020
 * partner: joowonk2
 */
#include "format.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define BILLION 1000000000L;

static void sig_handle(int); 
static volatile int status_flag = 0;

int main(int argc, char **argv) {
    // pid_t pid =  getpid();
    
    // char* pidstr = malloc(sizeof(char));
    // sprintf(pidstr, "%d", pid);
    // puts(pidstr);

    struct timespec start, stop;

    if( clock_gettime( CLOCK_MONOTONIC, &start) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }


    int iflag = 0;
    char *ivalue = NULL;

    int oflag = 0;
    char *ovalue = NULL;

    int bflag = 0;
    size_t blocksize = 512;   // default
                         
    int cflag = 0;
    int cvalue = 0;     // default (entire file)

    int pflag = 0;
    size_t pvalue = 0;     // default 

    int kflag = 0;
    size_t kvalue = 0;     // default

    int c;
    opterr = 0;

    // parse args
    while ((c = getopt (argc, argv, "i:o:b:c:p:k:")) != -1)
        switch (c) {
            case 'i':
                iflag = 1;
                ivalue = strdup(optarg);
                break;
            case 'o':
                oflag = 1;
                ovalue = strdup(optarg);
                break;
            case 'b':
                bflag = 1;
                blocksize = atoi(optarg);
                break;
            case 'c':
                cflag = 1;
                cvalue = atoi(optarg);
                break;
            case 'p':
                pflag = 1;
                pvalue = atoi(optarg);
                break;
            case 'k':
                kflag = 1;
                kvalue = atoi(optarg);
                break;
            case '?':
                return 1;
            default:
                abort();
            }

    
    // open files (or use stdin/out)
    FILE* input_file = stdin;
    FILE* output_file = stdout;

    if (iflag) {
        input_file = fopen(ivalue, "r");
        if (!input_file) {
            print_invalid_input(ivalue);   
            return 1;
        }
    } 

    if (oflag) {
        output_file = fopen(ovalue, "w+");
        if (!output_file) {
            print_invalid_output(ovalue);
            return 1;
        }
    }

    char str[1024 * 1024] = "";
    int s;

    size_t byte_count = 0;

    size_t fullblock_in = 0;
    size_t partialblock_in = 0;
    size_t fullblock_out = 0;
    size_t partialblock_out = 0;
    size_t bytesread = 0;
        
    if (signal(SIGUSR1, sig_handle) == SIG_ERR) {
        puts("can't handle");
    }

    // read and write
    while (1) {

        if (status_flag) {
            if ( clock_gettime( CLOCK_MONOTONIC, &stop) == -1 ) {
                perror( "clock gettime" );
                exit( EXIT_FAILURE );
            }
            double mid_time = (( stop.tv_sec + stop.tv_nsec ) / 1000000000) - (( start.tv_sec + start.tv_nsec ) / 1000000000);

            print_status_report(fullblock_in, partialblock_in, fullblock_out, partialblock_out, bytesread, mid_time);
            status_flag = 0;

        }
        size_t len = strlen(str);
        s = fgetc(input_file);
        if (feof(input_file)) {
            break;
        }
                
        snprintf(str + len, sizeof str - len, "%c", s);
        byte_count++;

        // break if at num blocks?
        if (cvalue != 0  && byte_count >= blocksize * cvalue) break;


        char buffer[strlen(str)];

        /* Write data to the file */
        fullblock_out = fwrite(str, strlen(str) + 1, (blocksize * pvalue) +1, output_file);

        /* Seek to the beginning of the file */
        fseek(output_file, blocksize * kvalue, SEEK_SET);

        /* Read and display data */
        bytesread += fread(buffer, strlen(str)+1, 1, output_file);

    }
    fullblock_in = bytesread / blocksize;
    if (bytesread % blocksize != 0) {
        partialblock_in++;
    }

    fullblock_out = bytesread / blocksize;
    if (bytesread % blocksize != 0) {
        partialblock_out++;
    }
    

    // get time
    if ( clock_gettime( CLOCK_MONOTONIC, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }

    printf("start: %ld\n\n", start.tv_sec);    
    printf("stop: %ld\n", stop.tv_sec);
    // printf("stop: %ld\n", stop.tv_nsec);
    // printf("start: %ld\n", start.tv_nsec);

    // TODO: why is this always zero?? stop.tv_sec = start.tv_sec ???
    long total_time = (( stop.tv_sec + stop.tv_nsec ) / 1000000000L) - (( start.tv_sec + start.tv_nsec ) / 1000000000L);

    // print status
    print_status_report(fullblock_in, partialblock_in, fullblock_out, partialblock_out, bytesread, total_time);

    // cleanup
    if (ivalue) {
        free(ivalue);
        ivalue = NULL;
    }
    // if (ovalue) {
    //     free(ovalue);
    //     ovalue = NULL;
    // }
    fclose(output_file);

    return 0;

}

void sig_handle(int sig) {
    puts("handling");
        if (sig == SIGUSR1) {
            // printf("received SIGUSR1\n");
            // raise flag - if flag is raised inside while print status report and lower flag
            status_flag = 1;
        }

}