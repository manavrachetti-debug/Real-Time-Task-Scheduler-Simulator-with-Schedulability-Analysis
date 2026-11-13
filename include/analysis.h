#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "validation.h"

typedef enum {
    POLICY_RMS = 0,
    POLICY_EDF = 1
} SchedPolicy;

typedef enum {
    VERDICT_GUARANTEED = 0,
    VERDICT_INCONCLUSIVE,
    VERDICT_NOT_SCHEDULABLE
} SchedVerdict;

typedef struct {
    double       utilization;
    double       rms_bound;
    int          hyperperiod_ms;
    SchedVerdict verdict;
} SchedAnalysis;

int gcd_int(int a, int b);

double calculate_utilization(const TaskSet *set);

double rms_utilization_bound(int task_count);

StatusCode calculate_hyperperiod(const TaskSet *set, int *hyperperiod_ms);

StatusCode analyze_schedulability(const TaskSet *set, SchedPolicy policy, SchedAnalysis *result);

const char *policy_name(SchedPolicy policy);

const char *verdict_name(SchedVerdict verdict);

#endif