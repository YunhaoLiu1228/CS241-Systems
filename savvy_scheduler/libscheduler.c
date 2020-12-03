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

#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

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
    int next_t;
    int total_required_t;
    int required_t;

    int remaining_t;
    int roundr_t;
    int end_t;
    
} job_info;


void scheduler_start_up(scheme_t s) {
   // LOG("start up");
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

    if (ja->required_t == jb->required_t) return break_tie(a, b);
    else if (ja->required_t < jb->required_t) return -1;
    else return 1;
}

int comparer_rr(const void *a, const void *b) {
    job_info* ja = (((job*) a)->metadata);
    job_info* jb = (((job*) b)->metadata);

    if (ja->next_t == jb->next_t) return break_tie(a, b);
    else if (ja->next_t < jb->next_t) return -1;
    else return 1;
}

int comparer_sjf(const void *a, const void *b) {
    job_info* ja = (((job*) a)->metadata);
    job_info* jb = (((job*) b)->metadata);

    if (ja->required_t == jb->required_t) return break_tie(a, b);
    else if (ja->required_t < jb->required_t) return -1;
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
  //  LOG("new job");
    
    job_info* info = malloc(sizeof(job_info));
    info->id = job_number;
    info->arrival_t = time;

    info->start_t = -1;
    info->next_t = -1;

    info->priority = sched_data->priority;
    info->required_t = sched_data->running_time;
    info->total_required_t = sched_data->running_time;

    newjob->metadata = info;

    priqueue_offer(&pqueue, newjob);
   // num_jobs_++;
    
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
  //  LOG("quantum expired");
// TODO: how do i know if there are waiting threads
/* * - the current scheme is non-preemptive and job_evicted is not NULL, return
 *   job_evicted.
 * - the current scheme is preemptive and job_evicted is not NULL, place
 *   job_evicted back on the queue and return the next job that should be ran.
 */
    if (job_evicted) {
        job_info* info = (job_info*) job_evicted->metadata;

        info->next_t = time;
        info->required_t--;

        if (info->start_t == -1) info->start_t = time -1;

        if (pqueue_scheme == PPRI || pqueue_scheme == PSRTF || pqueue_scheme == RR) {
            job* nextjob = priqueue_poll(&pqueue);
            
            priqueue_offer(&pqueue, (void *) nextjob);
            return (job*) priqueue_peek(&pqueue);
        } else {
            return job_evicted;
        }

    }

    else {
        return (job*) priqueue_peek(&pqueue);
    }
    
}

void scheduler_job_finished(job *job_done, double time) {
    job_info* info = job_done->metadata;
    //info->end_t = time;
    
    waiting_t_ += (time - info->arrival_t - info->total_required_t);
    turnaround_t_ += (time - info->arrival_t);
    response_t_ += (info->start_t - info->arrival_t);

    num_jobs_++;
    // free metadata
 //   free(info);

    priqueue_poll(&pqueue);
//LOG("job finished");
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    
    return  (waiting_t_ / (float)num_jobs_);
}

double scheduler_average_turnaround_time() {
    return (turnaround_t_ / (float)num_jobs_);
}

double scheduler_average_response_time() {
    return (response_t_ / (float)num_jobs_);
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
