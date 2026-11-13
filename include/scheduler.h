#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "validation.h"
#include "analysis.h"
#include "priority_queue.h"

#define IDLE_TASK_INDEX (-1)

typedef struct {
    int  task_index;
    int  jobs_released;
    int  jobs_completed;
    int  deadline_misses;
    long total_response_ms;
    int  max_response_ms;
} TaskStats;

typedef struct {
    SchedPolicy policy;
    int         simulated_ms;
    int         task_count;
    int        *timeline;
    TaskStats  *stats;
    int         busy_ms;
    int         idle_ms;
    int         context_switches;
    int         total_deadline_misses;
} SimulationResult;

int compare_rms(const Job *a, const Job *b);

int compare_edf(const Job *a, const Job *b);

JobCompare get_comparator(SchedPolicy policy);

StatusCode simulate_schedule(const TaskSet *set, SchedPolicy policy,
                             SimulationResult *result);

void free_simulation_result(SimulationResult *result);

#endif