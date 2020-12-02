/**
 * savvy_scheduler
 * CS 241 - Fall 2020
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

int num_jobs_;
int response_t_;
int turnaround_t_;
int waiting_t_;

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;
    int priority;
    int arrival_t;
    int start_t;
    int running_t;
    int remaining_t;
    int roundr_t;
    int end_t;
    
} job_info;


void scheduler_start_up(scheme_t s) {

    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
    num_jobs_ = 0;
    waiting_t_ = 0;
    turnaround_t_ = 0;
    response_t_ = 0;
    
}

/*
 * The following comparers can return -1, 0, or 1 based on the following:
 *  -1 if 'a' comes before 'b' in the ordering.
 *  1 if 'b' comes before 'a' in the ordering.
 *  0 if break_tie() returns 0.
 */

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    job_info* ja = (((job*) a)->metadata);
    job_info* jb = (((job*) b)->metadata);

    if (ja->arrival_t == jb->arrival_t) return 0;
    else if (ja->arrival_t < jb->arrival_t) return -1;
    else return 1;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

// lower priority number runs first
int comparer_pri(const void *a, const void *b) {
    job_info* ja = (((job*) a)->metadata);
    job_info* jb = (((job*) b)->metadata);

    if (ja->priority == jb->priority) return break_tie(a, b);
    else if (ja->priority < jb->priority) return -1;
    else return 1;
    
}

int comparer_psrtf(const void *a, const void *b) {
    job_info* ja = (((job*) a)->metadata);
    job_info* jb = (((job*) b)->metadata);

    if (ja->remaining_t == jb->remaining_t) return break_tie(a, b);
    else if (ja->remaining_t < jb->remaining_t) return -1;
    else return 1;
}

int comparer_rr(const void *a, const void *b) {
    job_info* ja = (((job*) a)->metadata);
    job_info* jb = (((job*) b)->metadata);

    if (ja->roundr_t == jb->roundr_t) return break_tie(a, b);
    else if (ja->roundr_t < jb->roundr_t) return -1;
    else return 1;
}

int comparer_sjf(const void *a, const void *b) {
    job_info* ja = (((job*) a)->metadata);
    job_info* jb = (((job*) b)->metadata);

    if (ja->running_t == jb->running_t) return break_tie(a, b);
    else if (ja->running_t < jb->running_t) return -1;
    else return 1;
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
//FCFS,  // First Come First Served
    // PPRI,  // Preemptive Priority
    // PRI,   // Priority
    // PSRTF, // Preemptive Least Remaining Time First
    // RR,    // Round Robin
    // SJF 
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    
    job_info* info = malloc(sizeof(job_info));
    info->id = job_number;
    info->arrival_t = time;
    info->priority = sched_data->priority;
    info->running_t = sched_data->running_time;

    newjob->metadata = info;

    priqueue_offer(&pqueue, newjob);

    // if (pqueue_scheme == FCFS) {
    //     printf("FCFS\n");
    // }

    // else if (pqueue_scheme == PPRI) {

    // } 
    
    // else if (pqueue_scheme == PRI) {

    // }

    // else if (pqueue_scheme == PSRTF) {

    // }

    // else if (pqueue_scheme == RR) {

    // }

    // else if (pqueue_scheme == SJF) {

    // }

    // else {
    //     fprintf(stdout, "Invalid scheme\n");
    //     exit(1);
    // }
    
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
// TODO: how do i know if there are waiting threads
/* * - the current scheme is non-preemptive and job_evicted is not NULL, return
 *   job_evicted.
 * - the current scheme is preemptive and job_evicted is not NULL, place
 *   job_evicted back on the queue and return the next job that should be ran.
 */
    if (job_evicted) {

        if (pqueue_scheme == PPRI || pqueue_scheme == PSRTF) {
            job* nextjob = priqueue_poll(&pqueue);
            priqueue_offer(&pqueue, (void *) job_evicted);
            return nextjob;
        } else {
            return job_evicted;
        }

    }

    else return NULL;
    
}

void scheduler_job_finished(job *job_done, double time) {
    
    job_info* info = job_done->metadata;
    info->end_t = time;
    
    waiting_t_ += info->start_t - info->arrival_t;
    turnaround_t_ += info->end_t - info->arrival_t;
    response_t_ += info->end_t - info->start_t;

}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    
    return (double) (waiting_t_ / num_jobs_);
}

double scheduler_average_turnaround_time() {
    return (double) (turnaround_t_ / num_jobs_);
}

double scheduler_average_response_time() {
    return (double) (response_t_ / num_jobs_);
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    while (priqueue_size(&pqueue) != 0) {
        free(priqueue_poll(&pqueue));
    }
    priqueue_destroy(&pqueue);
    print_stats();
}
