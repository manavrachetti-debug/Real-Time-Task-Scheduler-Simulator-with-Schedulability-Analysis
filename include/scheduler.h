#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "priority_queue.h"

// Define the base Task structure read from CSV
typedef struct {
    int id;
    int period;
    int execution_time;
    int deadline;
} Task;

// Internal state structure used during simulation tracking
typedef struct {
    int task_id;
    int period;
    int execution_time;
    int deadline;
    int remaining_time;
    int next_release;
} TaskState;

// Simulation loop function prototype
void run_simulation(const Task tasks[], int task_count, int hyperperiod, SchedPolicy policy, const char *output_filename);

#endif