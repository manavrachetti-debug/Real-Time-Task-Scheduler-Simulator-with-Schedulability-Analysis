#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_handler.h"

static const char CSV_HEADER[] = "id,name,period_ms,exec_time_ms";

static char *trim_in_place(char *text)
{
    char *end;

    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }

    if (*text == '\0') {
        return text;
    }

    end = text + strlen(text) - 1;
    while (end > text && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return text;
}

StatusCode task_set_init(TaskSet *set)
{
    if (set == NULL) {
        return ERR_NULL_POINTER;
    }

    set->count = 0;
    set->capacity = 0;
    set->tasks = (Task *)malloc((size_t)INITIAL_TASK_CAPACITY * sizeof(Task));

    if (set->tasks == NULL) {
        return ERR_MEMORY;
    }

    set->capacity = INITIAL_TASK_CAPACITY;
    return STATUS_OK;
}

StatusCode task_set_add(TaskSet *set, const Task *task)
{
    StatusCode status;
    Task *grown;
    int new_capacity;
    int i;

    if (set == NULL || task == NULL || set->tasks == NULL) {
        return ERR_NULL_POINTER;
    }

    status = validate_task(task);
    if (status != STATUS_OK) {
        return status;
    }

    if (set->count >= MAX_TASKS) {
        return ERR_TASK_LIMIT;
    }

    for (i = 0; i < set->count; i++) {
        if (set->tasks[i].id == task->id) {
            return ERR_DUPLICATE_ID;
        }
    }

    if (set->count >= set->capacity) {
        new_capacity = set->capacity * 2;
        if (new_capacity < INITIAL_TASK_CAPACITY) {
            new_capacity = INITIAL_TASK_CAPACITY;
        }
        if (new_capacity > MAX_TASKS) {
            new_capacity = MAX_TASKS;
        }

        grown = (Task *)realloc(set->tasks, (size_t)new_capacity * sizeof(Task));
        if (grown == NULL) {
            return ERR_MEMORY;
        }

        set->tasks = grown;
        set->capacity = new_capacity;
    }

    set->tasks[set->count] = *task;
    set->count++;
    return STATUS_OK;
}

void task_set_free(TaskSet *set)
{
    if (set == NULL) {
        return;
    }

    free(set->tasks);
    set->tasks = NULL;
    set->count = 0;
    set->capacity = 0;
}

StatusCode parse_task_line(const char *line, Task *task)
{
    char buffer[MAX_LINE_LEN];
    char *fields[4];
    char *cursor;
    char *comma;
    char *name;
    int comma_count = 0;
    int id;
    int period;
    int exec_time;
    int i;
    StatusCode status;
    Task parsed;

    if (line == NULL || task == NULL) {
        return ERR_NULL_POINTER;
    }

    if (strlen(line) >= MAX_LINE_LEN) {
        return ERR_FILE_FORMAT;
    }

    snprintf(buffer, sizeof buffer, "%s", line);

    for (cursor = buffer; *cursor != '\0'; cursor++) {
        if (*cursor == ',') {
            comma_count++;
        }
    }

    if (comma_count != 3) {
        return ERR_FILE_FORMAT;
    }

    cursor = buffer;
    for (i = 0; i < 4; i++) {
        fields[i] = cursor;
        if (i < 3) {
            comma = strchr(cursor, ',');
            if (comma == NULL) {
                return ERR_FILE_FORMAT;
            }
            *comma = '\0';
            cursor = comma + 1;
        }
    }

    if (parse_int_strict(trim_in_place(fields[0]), &id) != STATUS_OK) {
        return ERR_INVALID_NUMBER;
    }

    if (parse_int_strict(trim_in_place(fields[2]), &period) != STATUS_OK) {
        return ERR_INVALID_NUMBER;
    }

    if (parse_int_strict(trim_in_place(fields[3]), &exec_time) != STATUS_OK) {
        return ERR_INVALID_NUMBER;
    }

    name = trim_in_place(fields[1]);
    status = validate_task_name(name);
    if (status != STATUS_OK) {
        return status;
    }

    memset(&parsed, 0, sizeof parsed);
    parsed.id = id;
    memcpy(parsed.name, name, strlen(name) + 1);
    parsed.period_ms = period;
    parsed.exec_time_ms = exec_time;

    status = validate_task(&parsed);
    if (status != STATUS_OK) {
        return status;
    }

    *task = parsed;
    return STATUS_OK;
}

StatusCode load_tasks_from_csv(const char *filename, TaskSet *set,
                               int *error_line)
{
    FILE *fp;
    char line[MAX_LINE_LEN];
    char *start;
    size_t length;
    int line_number = 0;
    StatusCode status = STATUS_OK;
    Task task;

    if (error_line != NULL) {
        *error_line = 0;
    }

    if (filename == NULL || set == NULL) {
        return ERR_NULL_POINTER;
    }

    if (set->tasks == NULL) {
        status = task_set_init(set);
        if (status != STATUS_OK) {
            return status;
        }
    } else {
        set->count = 0;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        return ERR_FILE_OPEN;
    }

    while (status == STATUS_OK && fgets(line, sizeof line, fp) != NULL) {
        line_number++;
        length = strlen(line);

        if (length > 0 && line[length - 1] != '\n' && !feof(fp)) {
            status = ERR_FILE_FORMAT;
            break;
        }

        start = line;
        if (line_number == 1 &&
            (unsigned char)start[0] == 0xEF &&
            (unsigned char)start[1] == 0xBB &&
            (unsigned char)start[2] == 0xBF) {
            start += 3;
        }

        while (*start != '\0' && isspace((unsigned char)*start)) {
            start++;
        }

        if (*start == '\0' || *start == '#') {
            continue;
        }

        if (strncmp(start, CSV_HEADER, strlen(CSV_HEADER)) == 0) {
            continue;
        }

        status = parse_task_line(start, &task);
        if (status == STATUS_OK) {
            status = task_set_add(set, &task);
        }
    }

    if (status == STATUS_OK && ferror(fp)) {
        status = ERR_FILE_FORMAT;
    }

    fclose(fp);

    if (status != STATUS_OK) {
        set->count = 0;
        if (error_line != NULL) {
            *error_line = line_number;
        }
        return status;
    }

    if (set->count == 0) {
        return ERR_EMPTY_TASKSET;
    }

    return STATUS_OK;
}

StatusCode save_tasks_to_csv(const char *filename, const TaskSet *set)
{
    FILE *fp;
    int i;
    int failed = 0;

    if (filename == NULL || set == NULL) {
        return ERR_NULL_POINTER;
    }

    if (set->count <= 0) {
        return ERR_EMPTY_TASKSET;
    }

    if (set->tasks == NULL) {
        return ERR_NULL_POINTER;
    }

    fp = fopen(filename, "w");
    if (fp == NULL) {
        return ERR_FILE_OPEN;
    }

    if (fprintf(fp, "%s\n", CSV_HEADER) < 0) {
        failed = 1;
    }

    for (i = 0; i < set->count && !failed; i++) {
        if (fprintf(fp, "%d,%s,%d,%d\n",
                    set->tasks[i].id,
                    set->tasks[i].name,
                    set->tasks[i].period_ms,
                    set->tasks[i].exec_time_ms) < 0) {
            failed = 1;
        }
    }

    if (fclose(fp) == EOF) {
        failed = 1;
    }

    return failed ? ERR_FILE_WRITE : STATUS_OK;
}