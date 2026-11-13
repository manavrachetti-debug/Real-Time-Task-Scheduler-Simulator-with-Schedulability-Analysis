#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "analysis.h"

#define MAX_HYPERPERIOD_MS 100000

int gcd_int(int a, int b) {
    int temp;

    if (a < 0) {
        a = -a;
    }
    if (b < 0) {
        b = -b;
    }

    while (b != 0) {
        temp = a % b;
        a = b;
        b = temp;
    }

    return a;
}

double calculate_utilization(const TaskSet *set) {
    double total = 0.0;
    int i;

    if (set == NULL || set->tasks == NULL || set->count <= 0) {
        return 0.0;
    }

    for (i = 0; i < set->count; i++) {
        if (set->tasks[i].period_ms > 0) {
            total += (double)set->tasks[i].exec_time_ms / (double)set->tasks[i].period_ms;
        }
    }

    return total;
}

double rms_utilization_bound(int task_count) {
    if (task_count <= 0) {
        return 0.0;
    }
    return (double)task_count * (pow(2.0, 1.0 / (double)task_count) - 1.0);
}

StatusCode calculate_hyperperiod(const TaskSet *set, int *hyperperiod_ms) {
    int lcm;
    int divisor;
    int period;
    int quotient;
    int i;

    if (set == NULL || hyperperiod_ms == NULL) {
        return ERR_NULL_POINTER;
    }
    if (set->count <= 0) {
        return ERR_EMPTY_TASKSET;
    }
    if (set->tasks == NULL) {
        return ERR_NULL_POINTER;
    }

    lcm = set->tasks[0].period_ms;
    if (lcm <= 0) {
        return ERR_INVALID_PERIOD;
    }

    for (i = 1; i < set->count; i++) {
        period = set->tasks[i].period_ms;
        if (period <= 0) {
            return ERR_INVALID_PERIOD;
        }

        divisor = gcd_int(lcm, period);
        if (divisor == 0) {
            return ERR_INVALID_PERIOD;
        }

        quotient = lcm / divisor;
        if (quotient > MAX_HYPERPERIOD_MS / period) {
            return ERR_HYPERPERIOD_TOO_LARGE;
        }
        lcm = quotient * period;
    }

    *hyperperiod_ms = lcm;
    return STATUS_OK;
}

StatusCode analyze_schedulability(const TaskSet *set, SchedPolicy policy, SchedAnalysis *result) {
    StatusCode status;
    SchedAnalysis out;
    const double eps = 1e-9;

    if (set == NULL || result == NULL) {
        return ERR_NULL_POINTER;
    }
    if (set->count <= 0) {
        return ERR_EMPTY_TASKSET;
    }
    if (set->tasks == NULL) {
        return ERR_NULL_POINTER;
    }

    status = calculate_hyperperiod(set, &out.hyperperiod_ms);
    if (status != STATUS_OK) {
        return status;
    }

    out.utilization = calculate_utilization(set);
    out.rms_bound = rms_utilization_bound(set->count);

    if (policy == POLICY_RMS) {
        if (out.utilization > 1.0 + eps) {
            out.verdict = VERDICT_NOT_SCHEDULABLE;
        } else if (out.utilization <= out.rms_bound + eps) {
            out.verdict = VERDICT_GUARANTEED;
        } else {
            out.verdict = VERDICT_INCONCLUSIVE;
        }
    } else if (policy == POLICY_EDF) {
        if (out.utilization <= 1.0 + eps) {
            out.verdict = VERDICT_GUARANTEED;
        } else {
            out.verdict = VERDICT_NOT_SCHEDULABLE;
        }
    } else {
        return ERR_INVALID_NUMBER;
    }

    *result = out;
    return STATUS_OK;
}

const char *policy_name(SchedPolicy policy) {
    switch (policy) {
        case POLICY_RMS:
            return "RMS (Rate Monotonic Scheduling)";
        case POLICY_EDF:
            return "EDF (Earliest Deadline First)";
        default:
            return "Unknown Policy";
    }
}

const char *verdict_name(SchedVerdict verdict) {
    switch (verdict) {
        case VERDICT_GUARANTEED:
            return "GUARANTEED SCHEDULABLE";
        case VERDICT_INCONCLUSIVE:
            return "INCONCLUSIVE (Run Simulation)";
        case VERDICT_NOT_SCHEDULABLE:
            return "NOT SCHEDULABLE (CPU Overloaded)";
        default:
            return "Unknown Verdict";
    }
}