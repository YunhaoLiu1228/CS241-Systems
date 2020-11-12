/**
 * deepfried_dd
 * CS 241 - Fall 2020
 * partner: joowonk2, sap3, jeb5
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

static int full_blocks_in = 0;
static int partial_blocks_in = 0;
static int full_blocks_out = 0;
static int partial_blocks_out = 0;
static int total_bytes_copied = 0;
//static int total_blocks_copied = -1;

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
    int blocksize = 512;   // default
                         
    int cflag = 0;
    int count = -1;     // default (entire file)

    int pflag = 0;
    size_t pvalue = 0;     // default 

    int kflag = 0;
    size_t kvalue = 0;     // default

    int c;
    opterr = 0;

    // parse args
    while ((c = getopt (argc, argv, "i:o:b:c:p:k:")) != -1) {
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
                count = atoi(optarg);
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

    if (pflag == 1) {
        puts("y\n");
        int bytes = pvalue * blocksize;
        if (fseek(input_file, bytes, SEEK_SET)) {
            exit(1);
        }
    }

    if (kflag == 1)  {
                puts("sy\n");
        int bytes = kvalue * blocksize;
        if (fseek(output_file, bytes, SEEK_SET)) {
            exit(1);
        }
    }
        
    if (signal(SIGUSR1, sig_handle) == SIG_ERR) {
        puts("can't handle");
    }

    // read and write
    char buffer[blocksize];
    memset(buffer, 0, blocksize);
    int blocks_copied = 0;
 
    while (1) {
        if (status_flag) {
            clock_gettime(CLOCK_REALTIME, &stop);
            double diff = (double) difftime(stop.tv_nsec, start.tv_nsec) / BILLION;
            print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out,total_bytes_copied, diff);
            status_flag = 0;
        }

        int bytes = fread(&buffer, 1, blocksize, input_file);
        total_bytes_copied += bytes;

        fwrite(buffer, 1, bytes, output_file);
        blocks_copied++;

        if (bytes != blocksize) {
            partial_blocks_out++;
            partial_blocks_in++;
            break;
        } else if (blocks_copied == count) {
            full_blocks_out++;
            full_blocks_in++;
            break;
        }

        full_blocks_in++;
        full_blocks_out++;
    }
    

    // get time
    if ( clock_gettime( CLOCK_MONOTONIC, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }

    // printf("start: %ld\n\n", start.tv_sec);    
    // printf("stop: %ld\n", stop.tv_sec);
    // printf("stop: %ld\n", stop.tv_nsec);
    // printf("start: %ld\n", start.tv_nsec);

    // TODO: why is this always zero?? stop.tv_sec = start.tv_sec ???
    double total_time =  (double) difftime(stop.tv_nsec, start.tv_nsec) / BILLION;

    // print status
    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out, total_bytes_copied, total_time);

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