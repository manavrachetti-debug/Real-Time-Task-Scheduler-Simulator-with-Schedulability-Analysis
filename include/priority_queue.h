#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

typedef enum {
    SCHED_RMS,
    SCHED_EDF
} SchedPolicy;

typedef struct {
    int task_id;
    int absolute_deadline;
    int remaining_time;
    int release_time;
} Job;

#define MAX_JOBS 100

// THIS IS THE MIN-HEAP STRUCTURE FROM YOUR DESIGN DOC
typedef struct {
    Job data[MAX_JOBS];
    int size;
    SchedPolicy policy;
} MinHeap;

// Function Prototypes
void heap_init(MinHeap *hp, SchedPolicy policy);
int heap_push(MinHeap *hp, Job job);
Job heap_pop(MinHeap *hp);
int is_higher_priority(Job j1, Job j2, SchedPolicy policy);

#endif