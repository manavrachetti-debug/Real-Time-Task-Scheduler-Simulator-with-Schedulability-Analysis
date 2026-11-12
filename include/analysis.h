#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "scheduler.h"

double calculate_utilization(const Task tasks[], int count);
int calculate_hyperperiod(const Task tasks[], int count);
int check_rms_bound(double utilization, int task_count);

#endif