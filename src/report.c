#include <stdio.h>
#include "report.h"

char task_symbol(int task_index) {
    if (task_index == IDLE_TASK_INDEX) {
        return '.';
    }
    if (task_index >= 0 && task_index < 26) {
        return (char)('A' + task_index);
    }
    return '?';
}

void write_task_table(FILE *out, const TaskSet *set) {
    int i;
    double util;

    if (out == NULL || set == NULL || set->tasks == NULL || set->count <= 0) {
        return;
    }

    fprintf(out, "TASK SET (%d task%s)\n", set->count, set->count == 1 ? "" : "s");
    fprintf(out, "+-----+------+------------------+------------+----------+-------------+\n");
    fprintf(out, "| Sym | ID   | Name             | Period(ms) | WCET(ms) | Utilization |\n");
    fprintf(out, "+-----+------+------------------+------------+----------+-------------+\n");

    for (i = 0; i < set->count; i++) {
        util = 0.0;
        if (set->tasks[i].period_ms > 0) {
            util = ((double)set->tasks[i].exec_time_ms / (double)set->tasks[i].period_ms) * 100.0;
        }
        fprintf(out, "|  %c  | %4d | %-16s | %10d | %8d | %10.2f%% |\n",
                task_symbol(i),
                set->tasks[i].id,
                set->tasks[i].name,
                set->tasks[i].period_ms,
                set->tasks[i].exec_time_ms,
                util);
    }

    fprintf(out, "+-----+------+------------------+------------+----------+-------------+\n\n");
}

void write_analysis_section(FILE *out, const SchedAnalysis *analysis) {
    if (out == NULL || analysis == NULL) {
        return;
    }

    fprintf(out, "SCHEDULABILITY ANALYSIS\n");
    fprintf(out, "-------------------------------------------------------------------------\n");
    fprintf(out, "  Total CPU Utilization : %.4f (%.2f%%)\n",
            analysis->utilization, analysis->utilization * 100.0);
    fprintf(out, "  RMS Liu-Layland Bound : %.4f (%.2f%%)\n",
            analysis->rms_bound, analysis->rms_bound * 100.0);
    fprintf(out, "  Hyperperiod           : %d ms\n", analysis->hyperperiod_ms);
    fprintf(out, "  Analytical Verdict    : %s\n", verdict_name(analysis->verdict));
    fprintf(out, "-------------------------------------------------------------------------\n\n");
}

void write_timeline_section(FILE *out, const TaskSet *set,
                            const SimulationResult *result) {
    int start;
    int end;
    int t;
    int i;

    if (out == NULL || set == NULL || result == NULL || result->timeline == NULL) {
        return;
    }

    fprintf(out, "EXECUTION TIMELINE (%s, 0..%d ms)\n",
            policy_name(result->policy), result->simulated_ms);
    fprintf(out, "  Legend: ");
    for (i = 0; i < set->count; i++) {
        fprintf(out, "%c=%s  ", task_symbol(i), set->tasks[i].name);
    }
    fprintf(out, ".=IDLE\n");
    fprintf(out, "-------------------------------------------------------------------------\n");

    for (start = 0; start < result->simulated_ms; start += TIMELINE_ROW_WIDTH) {
        end = start + TIMELINE_ROW_WIDTH;
        if (end > result->simulated_ms) {
            end = result->simulated_ms;
        }
        fprintf(out, "  [%4d..%4d) | ", start, end);
        for (t = start; t < end; t++) {
            fputc(task_symbol(result->timeline[t]), out);
        }
        fputc('\n', out);
    }

    fprintf(out, "-------------------------------------------------------------------------\n\n");
}

void write_statistics_section(FILE *out, const TaskSet *set,
                              const SimulationResult *result) {
    int i;
    double avg_resp;
    double cpu_util;

    if (out == NULL || set == NULL || result == NULL || result->stats == NULL) {
        return;
    }

    fprintf(out, "PER-TASK SIMULATION STATISTICS\n");
    fprintf(out, "+-----+------------------+----------+-----------+--------+----------+----------+\n");
    fprintf(out, "| Sym | Name             | Released | Completed | Misses | Avg Resp | Max Resp |\n");
    fprintf(out, "+-----+------------------+----------+-----------+--------+----------+----------+\n");

    for (i = 0; i < set->count && i < result->task_count; i++) {
        avg_resp = 0.0;
        if (result->stats[i].jobs_completed > 0) {
            avg_resp = (double)result->stats[i].total_response_ms /
                       (double)result->stats[i].jobs_completed;
        }
        fprintf(out, "|  %c  | %-16s | %8d | %9d | %6d | %7.2fms | %7dms |\n",
                task_symbol(i),
                set->tasks[i].name,
                result->stats[i].jobs_released,
                result->stats[i].jobs_completed,
                result->stats[i].deadline_misses,
                avg_resp,
                result->stats[i].max_response_ms);
    }

    fprintf(out, "+-----+------------------+----------+-----------+--------+----------+----------+\n");

    cpu_util = 0.0;
    if (result->simulated_ms > 0) {
        cpu_util = ((double)result->busy_ms / (double)result->simulated_ms) * 100.0;
    }

    fprintf(out, "  Summary: Busy=%dms | Idle=%dms (CPU=%.2f%%) | Context Switches=%d | Total Misses=%d\n\n",
            result->busy_ms,
            result->idle_ms,
            cpu_util,
            result->context_switches,
            result->total_deadline_misses);
}

StatusCode write_full_report(FILE *out, const TaskSet *set,
                             const SchedAnalysis *analysis,
                             const SimulationResult *result) {
    if (out == NULL || set == NULL || analysis == NULL || result == NULL) {
        return ERR_NULL_POINTER;
    }
    if (set->count <= 0 || set->tasks == NULL) {
        return ERR_EMPTY_TASKSET;
    }
    if (result->timeline == NULL || result->stats == NULL) {
        return ERR_NULL_POINTER;
    }

    fprintf(out, "=========================================================================\n");
    fprintf(out, "  REAL-TIME TASK SCHEDULER REPORT - %s\n", policy_name(result->policy));
    fprintf(out, "=========================================================================\n\n");

    write_task_table(out, set);
    write_analysis_section(out, analysis);
    write_timeline_section(out, set, result);
    write_statistics_section(out, set, result);

    if (ferror(out)) {
        return ERR_FILE_WRITE;
    }

    return STATUS_OK;
}

StatusCode save_report_to_file(const char *filename, const TaskSet *set,
                               const SchedAnalysis *analysis,
                               const SimulationResult *result) {
    FILE *fp;
    StatusCode status;
    int close_failed = 0;

    if (filename == NULL || set == NULL || analysis == NULL || result == NULL) {
        return ERR_NULL_POINTER;
    }

    fp = fopen(filename, "w");
    if (fp == NULL) {
        return ERR_FILE_OPEN;
    }

    status = write_full_report(fp, set, analysis, result);

    if (fclose(fp) == EOF) {
        close_failed = 1;
    }

    if (status != STATUS_OK) {
        return status;
    }

    return close_failed ? ERR_FILE_WRITE : STATUS_OK;
}