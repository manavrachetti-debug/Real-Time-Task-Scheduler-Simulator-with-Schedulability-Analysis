#ifndef REPORT_H
#define REPORT_H

#include "scheduler.h"

void generate_summary_report(const Task tasks[], int count, SchedPolicy policy, double utilization, int hyperperiod);

#endif