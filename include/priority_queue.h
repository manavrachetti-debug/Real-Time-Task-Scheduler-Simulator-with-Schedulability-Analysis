#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "validation.h"

typedef struct {
    int task_index;
    int job_number;
    int release_time;
    int absolute_deadline;
    int remaining_time;
    int period_ms;
} Job;

typedef int (*JobCompare)(const Job *a, const Job *b);

typedef struct {
    Job *items;
    int size;
    int capacity;
    JobCompare compare;
} PriorityQueue;

StatusCode pq_init(PriorityQueue *pq, int initial_capacity, JobCompare compare);

StatusCode pq_push(PriorityQueue *pq, const Job *job);

StatusCode pq_pop(PriorityQueue *pq, Job *out);

StatusCode pq_peek(const PriorityQueue *pq, Job *out);

int pq_is_empty(const PriorityQueue *pq);

int pq_size(const PriorityQueue *pq);

void pq_free(PriorityQueue *pq);

#endif