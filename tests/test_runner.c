#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "validation.h"
#include "file_handler.h"
#include "priority_queue.h"
#include "analysis.h"
#include "scheduler.h"
#include "report.h"

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_TRUE(expr, msg) do { \
    tests_run++; \
    if (expr) { \
        tests_passed++; \
        printf("[PASS] %s\n", msg); \
    } else { \
        printf("[FAIL] %s (Line %d)\n", msg, __LINE__); \
    } \
} while (0)

static int deadline_cmp(const Job *a, const Job *b) {
    if (a->absolute_deadline != b->absolute_deadline) {
        return (a->absolute_deadline < b->absolute_deadline) ? -1 : 1;
    }
    return 0;
}

static void test_validation_module(void) {
    int val = 0;
    Task valid_t = {1, "Sensor_1", 20, 5};
    Task bad_id = {0, "Sensor_1", 20, 5};
    Task bad_exec = {2, "Sensor_2", 10, 15};

    printf("\n--- Testing validation.c ---\n");
    ASSERT_TRUE(validate_task_name("Control_Loop_1") == STATUS_OK, "Valid task name passes");
    ASSERT_TRUE(validate_task_name("Bad-Name") == ERR_INVALID_NAME, "Hyphen in task name rejected");
    ASSERT_TRUE(parse_int_strict("42", &val) == STATUS_OK && val == 42, "Strict integer parse valid");
    ASSERT_TRUE(parse_int_strict("42a", &val) == ERR_INVALID_NUMBER, "Strict integer parse rejects trailing letters");
    ASSERT_TRUE(validate_task(&valid_t) == STATUS_OK, "Valid task struct passes");
    ASSERT_TRUE(validate_task(&bad_id) == ERR_INVALID_ID, "Zero task ID rejected");
    ASSERT_TRUE(validate_task(&bad_exec) == ERR_INVALID_EXEC_TIME, "WCET > period rejected");
}

static void test_file_handler_module(void) {
    Task parsed;
    TaskSet set;
    Task t1 = {1, "T1", 10, 2};
    Task dup = {1, "T2", 20, 4};

    printf("\n--- Testing file_handler.c ---\n");
    ASSERT_TRUE(parse_task_line("1, Motor_Ctrl , 50 , 10", &parsed) == STATUS_OK &&
                parsed.id == 1 && parsed.period_ms == 50 && parsed.exec_time_ms == 10,
                "Parse valid CSV line with spaces");
    ASSERT_TRUE(parse_task_line("1,Motor_Ctrl,50", &parsed) == ERR_FILE_FORMAT,
                "Reject CSV line with missing column");

    ASSERT_TRUE(task_set_init(&set) == STATUS_OK, "TaskSet initializes cleanly");
    ASSERT_TRUE(task_set_add(&set, &t1) == STATUS_OK, "Add valid task to TaskSet");
    ASSERT_TRUE(task_set_add(&set, &dup) == ERR_DUPLICATE_ID, "Reject duplicate task ID");
    task_set_free(&set);
}

static void test_priority_queue_module(void) {
    PriorityQueue pq;
    Job j1 = {0, 1, 0, 30, 2, 30};
    Job j2 = {1, 1, 0, 10, 1, 10};
    Job j3 = {2, 1, 0, 20, 3, 20};
    Job out;

    printf("\n--- Testing priority_queue.c ---\n");
    ASSERT_TRUE(pq_init(&pq, 2, deadline_cmp) == STATUS_OK, "PriorityQueue initializes");
    ASSERT_TRUE(pq_pop(&pq, &out) == ERR_HEAP_EMPTY, "Pop on empty queue returns ERR_HEAP_EMPTY");

    pq_push(&pq, &j1);
    pq_push(&pq, &j2);
    pq_push(&pq, &j3);
    ASSERT_TRUE(pq_size(&pq) == 3, "Heap grows dynamically past initial capacity");

    pq_pop(&pq, &out);
    ASSERT_TRUE(out.task_index == 1 && out.absolute_deadline == 10, "First pop returns earliest deadline");
    pq_pop(&pq, &out);
    ASSERT_TRUE(out.task_index == 2 && out.absolute_deadline == 20, "Second pop returns second deadline");
    pq_free(&pq);
}

static void test_analysis_and_scheduler(void) {
    TaskSet set;
    Task t1 = {1, "A", 10, 2};
    Task t2 = {2, "B", 20, 4};
    Task t3 = {3, "C", 40, 8};
    SchedAnalysis analysis;
    SimulationResult sim;

    printf("\n--- Testing analysis.c & scheduler.c ---\n");
    task_set_init(&set);
    task_set_add(&set, &t1);
    task_set_add(&set, &t2);
    task_set_add(&set, &t3);

    ASSERT_TRUE(fabs(calculate_utilization(&set) - 0.60) < 1e-6, "Harmonic utilization equals 0.60");
    ASSERT_TRUE(analyze_schedulability(&set, POLICY_RMS, &analysis) == STATUS_OK &&
                analysis.hyperperiod_ms == 40 && analysis.verdict == VERDICT_GUARANTEED,
                "RMS schedulability guaranteed for U=0.60, H=40");

    ASSERT_TRUE(simulate_schedule(&set, POLICY_EDF, &sim) == STATUS_OK &&
                sim.total_deadline_misses == 0 && sim.busy_ms == 24 && sim.idle_ms == 16,
                "EDF simulation completes 40ms hyperperiod with 0 misses");

    free_simulation_result(&sim);
    task_set_free(&set);
}

int main(void) {
    printf("=========================================================================\n");
    printf("  UNIT TEST SUITE - REAL-TIME TASK SCHEDULER SIMULATOR\n");
    printf("=========================================================================\n");

    test_validation_module();
    test_file_handler_module();
    test_priority_queue_module();
    test_analysis_and_scheduler();

    printf("\n=========================================================================\n");
    printf("  SUMMARY: %d / %d tests passed.\n", tests_passed, tests_run);
    printf("=========================================================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}