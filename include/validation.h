
#ifndef VALIDATION_H
#define VALIDATION_H

#define MAX_TASKS              20     
#define MAX_NAME_LEN           32      
#define MIN_TIME_MS            1       
#define MAX_PERIOD_MS          1000    
#define MAX_HYPERPERIOD_MS     10000   
#define INITIAL_TASK_CAPACITY  4      
typedef enum {
    STATUS_OK = 0,
    ERR_NULL_POINTER,
    ERR_INVALID_NUMBER,
    ERR_INVALID_NAME,
    ERR_INVALID_ID,
    ERR_INVALID_PERIOD,
    ERR_INVALID_EXEC_TIME,
    ERR_DUPLICATE_ID,
    ERR_TASK_LIMIT,
    ERR_EMPTY_TASKSET,
    ERR_FILE_OPEN,
    ERR_FILE_FORMAT,
    ERR_FILE_WRITE,
    ERR_MEMORY,
    ERR_HEAP_EMPTY,
    ERR_INVALID_POLICY,
    ERR_HYPERPERIOD_TOO_LARGE
} StatusCode;

typedef struct {
    int  id;                      /* unique positive id                  */
    char name[MAX_NAME_LEN];      /* letters, digits, underscore only    */
    int  period_ms;               /* release period in milliseconds      */
    int  exec_time_ms;            /* worst-case execution time (WCET)    */
} Task;

typedef struct {
    Task *tasks;                  /* heap-allocated array                */
    int   count;                  /* number of tasks stored              */
    int   capacity;               /* allocated slots                     */
} TaskSet;

StatusCode validate_task_name(const char *name);

StatusCode validate_task(const Task *task);

StatusCode validate_task_set(const TaskSet *set);

StatusCode parse_int_strict(const char *text, int *value);

const char *status_message(StatusCode code);

#endif