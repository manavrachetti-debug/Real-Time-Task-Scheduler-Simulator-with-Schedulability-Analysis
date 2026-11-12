#include "../include/scheduler.h"
#include "../include/analysis.h"
#include "../include/report.h"
#include "../include/file_handler.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_TASKS 20

int main(int argc, char *argv[]) {
    const char *csv_file = (argc > 1) ? argv[1] : "tasks.csv";
    Task tasks[MAX_TASKS];

    printf("Loading task set from %s...\n", csv_file);
    int task_count = load_tasks_from_csv(csv_file, tasks, MAX_TASKS);

    if (task_count <= 0) {
        printf("Failed to load tasks or empty file.\n");
        return 1;
    }

    double utilization = calculate_utilization(tasks, task_count);
    int hyperperiod = calculate_hyperperiod(tasks, task_count);

    // 1. Run Rate Monotonic Scheduling (RMS)
    generate_summary_report(tasks, task_count, SCHED_RMS, utilization, hyperperiod);
    run_simulation(tasks, task_count, hyperperiod, SCHED_RMS, "rms_simulation_log.txt");

    // 2. Run Earliest Deadline First (EDF)
    generate_summary_report(tasks, task_count, SCHED_EDF, utilization, hyperperiod);
    run_simulation(tasks, task_count, hyperperiod, SCHED_EDF, "edf_simulation_log.txt");

    return 0;
}