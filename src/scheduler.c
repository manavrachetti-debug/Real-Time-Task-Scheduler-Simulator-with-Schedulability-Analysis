#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

int compare_rms(const Job *a, const Job *b) {
    if (a->period_ms != b->period_ms) {
        return (a->period_ms < b->period_ms) ? -1 : 1;
    }
    if (a->release_time != b->release_time) {
        return (a->release_time < b->release_time) ? -1 : 1;
    }
    if (a->task_index != b->task_index) {
        return (a->task_index < b->task_index) ? -1 : 1;
    }
    return 0;
}

int compare_edf(const Job *a, const Job *b) {
    if (a->absolute_deadline != b->absolute_deadline) {
        return (a->absolute_deadline < b->absolute_deadline) ? -1 : 1;
    }
    if (a->release_time != b->release_time) {
        return (a->release_time < b->release_time) ? -1 : 1;
    }
    if (a->task_index != b->task_index) {
        return (a->task_index < b->task_index) ? -1 : 1;
    }
    return 0;
}

JobCompare get_comparator(SchedPolicy policy) {
    if (policy == POLICY_RMS) {
        return compare_rms;
    }
    if (policy == POLICY_EDF) {
        return compare_edf;
    }
    return NULL;
}

static void init_result_zero(SimulationResult *result) {
    memset(result, 0, sizeof(*result));
}

StatusCode simulate_schedule(const TaskSet *set, SchedPolicy policy,
                             SimulationResult *result) {
    JobCompare compare;
    StatusCode status;
    int hyperperiod_ms;
    PriorityQueue pq;
    int *job_counts = NULL;
    int t;
    int i;
    int prev_task = IDLE_TASK_INDEX;

    if (set == NULL || result == NULL) {
        return ERR_NULL_POINTER;
    }

    init_result_zero(result);

    status = validate_task_set(set);
    if (status != STATUS_OK) {
        return status;
    }

    compare = get_comparator(policy);
    if (compare == NULL) {
        return ERR_INVALID_POLICY;
    }

    status = calculate_hyperperiod(set, &hyperperiod_ms);
    if (status != STATUS_OK) {
        return status;
    }

    result->timeline = (int *)malloc((size_t)hyperperiod_ms * sizeof(int));
    if (result->timeline == NULL) {
        return ERR_MEMORY;
    }

    result->stats = (TaskStats *)calloc((size_t)set->count, sizeof(TaskStats));
    if (result->stats == NULL) {
        free(result->timeline);
        init_result_zero(result);
        return ERR_MEMORY;
    }

    job_counts = (int *)calloc((size_t)set->count, sizeof(int));
    if (job_counts == NULL) {
        free(result->stats);
        free(result->timeline);
        init_result_zero(result);
        return ERR_MEMORY;
    }

    for (i = 0; i < set->count; i++) {
        result->stats[i].task_index = i;
    }

    status = pq_init(&pq, set->count * 2, compare);
    if (status != STATUS_OK) {
        free(job_counts);
        free(result->stats);
        free(result->timeline);
        init_result_zero(result);
        return status;
    }

    result->policy = policy;
    result->simulated_ms = hyperperiod_ms;
    result->task_count = set->count;

    for (t = 0; t < hyperperiod_ms; t++) {
        for (i = 0; i < set->count; i++) {
            if (t % set->tasks[i].period_ms == 0) {
                Job new_job;
                job_counts[i]++;
                new_job.task_index = i;
                new_job.job_number = job_counts[i];
                new_job.release_time = t;
                new_job.absolute_deadline = t + set->tasks[i].period_ms;
                new_job.remaining_time = set->tasks[i].exec_time_ms;
                new_job.period_ms = set->tasks[i].period_ms;

                result->stats[i].jobs_released++;

                status = pq_push(&pq, &new_job);
                if (status != STATUS_OK) {
                    pq_free(&pq);
                    free(job_counts);
                    free_simulation_result(result);
                    return status;
                }
            }
        }

        if (pq_is_empty(&pq)) {
            result->timeline[t] = IDLE_TASK_INDEX;
            result->idle_ms++;
            prev_task = IDLE_TASK_INDEX;
        } else {
            Job current;
            int idx;
            int finish_time;
            int response_ms;

            status = pq_pop(&pq, &current);
            if (status != STATUS_OK) {
                pq_free(&pq);
                free(job_counts);
                free_simulation_result(result);
                return status;
            }

            idx = current.task_index;
            result->timeline[t] = idx;
            result->busy_ms++;

            if (prev_task != IDLE_TASK_INDEX && prev_task != idx) {
                result->context_switches++;
            }
            prev_task = idx;

            current.remaining_time--;

            if (current.remaining_time == 0) {
                finish_time = t + 1;
                response_ms = finish_time - current.release_time;

                result->stats[idx].jobs_completed++;
                result->stats[idx].total_response_ms += (long)response_ms;
                if (response_ms > result->stats[idx].max_response_ms) {
                    result->stats[idx].max_response_ms = response_ms;
                }

                if (finish_time > current.absolute_deadline) {
                    result->stats[idx].deadline_misses++;
                    result->total_deadline_misses++;
                }
            } else {
                status = pq_push(&pq, &current);
                if (status != STATUS_OK) {
                    pq_free(&pq);
                    free(job_counts);
                    free_simulation_result(result);
                    return status;
                }
            }
        }
    }

    while (!pq_is_empty(&pq)) {
        Job leftover;
        pq_pop(&pq, &leftover);
        result->stats[leftover.task_index].deadline_misses++;
        result->total_deadline_misses++;
    }

    pq_free(&pq);
    free(job_counts);
    return STATUS_OK;
}

void free_simulation_result(SimulationResult *result) {
    if (result == NULL) {
        return;
    }
    free(result->timeline);
    free(result->stats);
    init_result_zero(result);
}