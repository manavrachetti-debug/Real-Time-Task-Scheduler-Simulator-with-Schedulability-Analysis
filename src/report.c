#include "../include/report.h"
#include "../include/analysis.h"
#include <stdio.h>
#include <math.h>

void generate_summary_report(const Task tasks[], int count, SchedPolicy policy, double utilization, int hyperperiod) {
    printf("\n=========================================\n");
    printf("         SCHEDULABILITY REPORT           \n");
    printf("=========================================\n");
    printf("Scheduling Policy : %s\n", (policy == SCHED_RMS) ? "Rate Monotonic (RMS)" : "Earliest Deadline First (EDF)");
    printf("Total Tasks       : %d\n", count);
    printf("Hyperperiod       : %d ms\n", hyperperiod);
    printf("CPU Utilization   : %.2f%%\n", utilization * 100.0);

    if (policy == SCHED_RMS) {
        double bound = count * (pow(2.0, 1.0 / count) - 1.0);
        int passes = (utilization <= bound);
        printf("RMS Bound Limit   : %.2f%%\n", bound * 100.0);
        printf("Schedulability    : %s\n", passes ? "PASSED (Guaranteed Schedulable)" : "INCONCLUSIVE (Check simulation log for misses)");
    } else {
        int passes = (utilization <= 1.0);
        printf("Schedulability    : %s\n", passes ? "PASSED (Guaranteed Schedulable)" : "FAILED (CPU Overloaded > 100%)");
    }
    printf("=========================================\n");
}