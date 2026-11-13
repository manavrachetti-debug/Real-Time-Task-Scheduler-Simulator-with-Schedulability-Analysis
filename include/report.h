#ifndef REPORT_H
#define REPORT_H

#include <stdio.h>
#include "validation.h"
#include "analysis.h"
#include "scheduler.h"

#define TIMELINE_ROW_WIDTH 50

char task_symbol(int task_index);

void write_task_table(FILE *out, const TaskSet *set);

void write_analysis_section(FILE *out, const SchedAnalysis *analysis);

void write_timeline_section(FILE *out, const TaskSet *set,
                            const SimulationResult *result);

void write_statistics_section(FILE *out, const TaskSet *set,
                              const SimulationResult *result);

StatusCode write_full_report(FILE *out, const TaskSet *set,
                             const SchedAnalysis *analysis,
                             const SimulationResult *result);

StatusCode save_report_to_file(const char *filename, const TaskSet *set,
                               const SchedAnalysis *analysis,
                               const SimulationResult *result);

#endif