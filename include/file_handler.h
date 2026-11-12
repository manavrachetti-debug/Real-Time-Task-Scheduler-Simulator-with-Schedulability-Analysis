#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "scheduler.h"

// Loads tasks from a CSV file into the provided array. Returns task count or -1 on error.
int load_tasks_from_csv(const char *filename, Task tasks[], int max_tasks);

#endif