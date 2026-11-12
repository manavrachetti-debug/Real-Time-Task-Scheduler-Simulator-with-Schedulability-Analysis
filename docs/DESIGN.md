# Detailed Design Document: RTOS Task Scheduler Simulator

## 1. Data Dictionary & Structures

### Core Task & Job Structures
```c
typedef enum {
    SCHED_RMS,
    SCHED_EDF
} SchedPolicy;

typedef struct {
    int id;             // Task ID (positive integer)
    int period;         // Task period T_i (ms)
    int execution_time; // Execution time C_i (ms)
    int deadline;       // Relative deadline D_i (ms)
} Task;

typedef struct {
    int task_id;
    int absolute_deadline;
    int remaining_time;
    int release_time;
} Job;