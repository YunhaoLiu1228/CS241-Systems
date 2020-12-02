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

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;
    int arrival_t;
    int priority;
    int running_t;
    int remaining_t;
    int roundr_t;
    
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
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    
    job_info* info = malloc(sizeof(job_info));
    info->id = job_number;
    info->arrival_t = time;
    info->priority = sched_data->priority;
    info->running_t = sched_data->running_time;

    newjob->metadata = info;
    
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    return NULL;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return 9001;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return 9001;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    return 9001;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
