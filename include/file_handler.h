#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "validation.h"

#define MAX_LINE_LEN 256

StatusCode task_set_init(TaskSet *set);

StatusCode task_set_add(TaskSet *set, const Task *task);

void task_set_free(TaskSet *set);

StatusCode parse_task_line(const char *line, Task *task);

StatusCode load_tasks_from_csv(const char *filename, TaskSet *set,
                               int *error_line);

StatusCode save_tasks_to_csv(const char *filename, const TaskSet *set);

#endif