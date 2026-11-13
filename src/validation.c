#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "validation.h"

StatusCode validate_task_name(const char *name)
{
    size_t length;
    size_t i;

    if (name == NULL) {
        return ERR_NULL_POINTER;
    }

    length = strlen(name);
    if (length == 0 || length >= MAX_NAME_LEN) {
        return ERR_INVALID_NAME;
    }

    for (i = 0; i < length; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!isalnum(c) && c != '_') {
            return ERR_INVALID_NAME;
        }
    }

    return STATUS_OK;
}

StatusCode validate_task(const Task *task)
{
    StatusCode status;

    if (task == NULL) {
        return ERR_NULL_POINTER;
    }

    if (task->id <= 0) {
        return ERR_INVALID_ID;
    }

    if (memchr(task->name, '\0', MAX_NAME_LEN) == NULL) {
        return ERR_INVALID_NAME;
    }

    status = validate_task_name(task->name);
    if (status != STATUS_OK) {
        return status;
    }

    if (task->period_ms < MIN_TIME_MS || task->period_ms > MAX_PERIOD_MS) {
        return ERR_INVALID_PERIOD;
    }

    if (task->exec_time_ms < MIN_TIME_MS ||
        task->exec_time_ms > task->period_ms) {
        return ERR_INVALID_EXEC_TIME;
    }

    return STATUS_OK;
}

StatusCode validate_task_set(const TaskSet *set)
{
    int i;
    int j;
    StatusCode status;

    if (set == NULL) {
        return ERR_NULL_POINTER;
    }

    if (set->count <= 0) {
        return ERR_EMPTY_TASKSET;
    }

    if (set->count > MAX_TASKS) {
        return ERR_TASK_LIMIT;
    }

    if (set->tasks == NULL) {
        return ERR_NULL_POINTER;
    }

    for (i = 0; i < set->count; i++) {
        status = validate_task(&set->tasks[i]);
        if (status != STATUS_OK) {
            return status;
        }
    }

    for (i = 0; i < set->count; i++) {
        for (j = i + 1; j < set->count; j++) {
            if (set->tasks[i].id == set->tasks[j].id) {
                return ERR_DUPLICATE_ID;
            }
        }
    }

    return STATUS_OK;
}

StatusCode parse_int_strict(const char *text, int *value)
{
    char *end = NULL;
    long parsed;

    if (text == NULL || value == NULL) {
        return ERR_NULL_POINTER;
    }

    while (isspace((unsigned char)*text)) {
        text++;
    }

    if (*text == '\0') {
        return ERR_INVALID_NUMBER;
    }

    errno = 0;
    parsed = strtol(text, &end, 10);

    if (end == text) {
        return ERR_INVALID_NUMBER;
    }

    if (errno == ERANGE || parsed > INT_MAX || parsed < INT_MIN) {
        return ERR_INVALID_NUMBER;
    }

    while (isspace((unsigned char)*end)) {
        end++;
    }

    if (*end != '\0') {
        return ERR_INVALID_NUMBER;
    }

    *value = (int)parsed;
    return STATUS_OK;
}

const char *status_message(StatusCode code)
{
    switch (code) {
    case STATUS_OK:
        return "Success";
    case ERR_NULL_POINTER:
        return "Internal error: null pointer";
    case ERR_INVALID_NUMBER:
        return "Invalid number";
    case ERR_INVALID_NAME:
        return "Invalid task name (use letters, digits, underscore; max 31 chars)";
    case ERR_INVALID_ID:
        return "Invalid task id (must be a positive integer)";
    case ERR_INVALID_PERIOD:
        return "Invalid period (must be between 1 and 1000 ms)";
    case ERR_INVALID_EXEC_TIME:
        return "Invalid execution time (must be at least 1 ms and not more than the period)";
    case ERR_DUPLICATE_ID:
        return "Duplicate task id";
    case ERR_TASK_LIMIT:
        return "Too many tasks (maximum is 20)";
    case ERR_EMPTY_TASKSET:
        return "Task set is empty";
    case ERR_FILE_OPEN:
        return "Could not open file";
    case ERR_FILE_FORMAT:
        return "Invalid file format";
    case ERR_FILE_WRITE:
        return "Could not write to file";
    case ERR_MEMORY:
        return "Out of memory";
    case ERR_HEAP_EMPTY:
        return "Priority queue is empty";
    case ERR_INVALID_POLICY:
        return "Invalid scheduling policy";
    case ERR_HYPERPERIOD_TOO_LARGE:
        return "Hyperperiod is too large (maximum is 10000 ms); use harmonic periods";
    default:
        return "Unknown error";
    }
}