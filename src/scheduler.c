#include "../include/scheduler.h"
#include "../include/priority_queue.h"
#include <stdio.h>
#include <stdlib.h>

void run_simulation(const Task tasks[], int task_count, int hyperperiod, SchedPolicy policy, const char *output_filename) {
    FILE *fp = fopen(output_filename, "w");
    if (!fp) {
        printf("Error: Could not open output file %s\n", output_filename);
        return;
    }

    fprintf(fp, "=== Real-Time Task Scheduler Simulation Log ===\n");
    fprintf(fp, "Policy: %s\n", (policy == SCHED_RMS) ? "Rate Monotonic Scheduling (RMS)" : "Earliest Deadline First (EDF)");
    fprintf(fp, "Hyperperiod: %d ms\n\n", hyperperiod);

    // Initialize task states dynamically for simulation tracking
    TaskState *states = malloc(task_count * sizeof(TaskState));
    for (int i = 0; i < task_count; i++) {
        states[i].task_id = tasks[i].id;
        states[i].period = tasks[i].period;
        states[i].execution_time = tasks[i].execution_time;
        states[i].deadline = tasks[i].deadline;
        states[i].remaining_time = 0;
        states[i].next_release = 0;
    }

    MinHeap pq;
    heap_init(&pq, policy);

    int deadline_misses = 0;

    // Discrete time simulation loop from t = 0 up to the Hyperperiod
    for (int t = 0; t < hyperperiod; t++) {
        // 1. Check for job releases at current time t
        for (int i = 0; i < task_count; i++) {
            if (t == states[i].next_release) {
                // If a job was still active and not finished, it's a deadline miss!
                if (states[i].remaining_time > 0) {
                    fprintf(fp, "[Time %dms] WARNING: Task %d missed its deadline!\n", t, states[i].task_id);
                    deadline_misses++;
                }

                // Release new job instance
                Job new_job;
                new_job.task_id = states[i].task_id;
                new_job.release_time = t;
                new_job.absolute_deadline = t + states[i].deadline;
                new_job.remaining_time = states[i].execution_time;

                states[i].remaining_time = states[i].execution_time;
                states[i].next_release += states[i].period;

                heap_push(&pq, new_job);
            }
        }

        // 2. Execute highest priority job for 1 time unit (if any available)
        if (pq.size > 0) {
            // Pop root (highest priority)
            Job current_job = heap_pop(&pq);
            
            fprintf(fp, "[Time %dms] Executing Task %d (Remaining: %dms)\n", t, current_job.task_id, current_job.remaining_time);
            current_job.remaining_time--;

            // If job still needs execution time, push it back into the heap
            if (current_job.remaining_time > 0) {
                heap_push(&pq, current_job);
            }
        } else {
            fprintf(fp, "[Time %dms] CPU Idle\n", t);
        }
    }

    fprintf(fp, "\nSimulation completed. Total deadline misses: %d\n", deadline_misses);
    fclose(fp);
    free(states);
    printf("Simulation successfully finished! Results logged to %s\n", output_filename);
}