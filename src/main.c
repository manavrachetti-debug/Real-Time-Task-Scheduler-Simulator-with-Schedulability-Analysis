#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "validation.h"
#include "file_handler.h"
#include "analysis.h"
#include "scheduler.h"
#include "report.h"

#define INPUT_BUF_LEN 256
#define DEFAULT_CSV_PATH "tasks.csv"

static void read_line(char *buf, size_t size) {
    size_t len;

    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }

    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[len - 1] = '\0';
        len--;
    }
}

static char *trim_whitespace(char *str) {
    char *end;

    while (*str != '\0' && isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == '\0') {
        return str;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

static void print_banner(void) {
    printf("=========================================================================\n");
    printf("   REAL-TIME TASK SCHEDULER SIMULATOR & SCHEDULABILITY ANALYZER\n");
    printf("=========================================================================\n");
}

static void print_menu(int task_count) {
    printf("\n-------------------------------------------------------------------------\n");
    printf("  Current Task Set: %d task%s loaded\n", task_count, task_count == 1 ? "" : "s");
    printf("-------------------------------------------------------------------------\n");
    printf("  1. Load tasks from CSV file\n");
    printf("  2. Add a task manually\n");
    printf("  3. View current tasks\n");
    printf("  4. Clear all tasks\n");
    printf("  5. Save current tasks to CSV file\n");
    printf("  6. Run RMS (Rate Monotonic) analysis & simulation\n");
    printf("  7. Run EDF (Earliest Deadline First) analysis & simulation\n");
    printf("  8. Compare RMS vs EDF side-by-side\n");
    printf("  0. Exit\n");
    printf("-------------------------------------------------------------------------\n");
    printf("  Enter choice: ");
}

static void handle_load_csv(TaskSet *set) {
    char buf[INPUT_BUF_LEN];
    char *path;
    int error_line = 0;
    StatusCode status;

    printf("  Enter CSV file path [default: %s]: ", DEFAULT_CSV_PATH);
    read_line(buf, sizeof(buf));
    path = trim_whitespace(buf);

    if (path[0] == '\0') {
        path = DEFAULT_CSV_PATH;
    }

    status = load_tasks_from_csv(path, set, &error_line);
    if (status == STATUS_OK) {
        printf("  [OK] Loaded %d task%s from '%s'.\n",
               set->count, set->count == 1 ? "" : "s", path);
    } else if (error_line > 0) {
        printf("  [ERROR] %s (Line %d): %s\n", path, error_line, status_message(status));
    } else {
        printf("  [ERROR] Failed to load '%s': %s\n", path, status_message(status));
    }
}

static void handle_add_task(TaskSet *set) {
    char buf[INPUT_BUF_LEN];
    char *trimmed;
    Task task;
    StatusCode status;

    memset(&task, 0, sizeof(task));

    printf("  Task ID (1..9999): ");
    read_line(buf, sizeof(buf));
    status = parse_int_strict(trim_whitespace(buf), &task.id);
    if (status != STATUS_OK) {
        printf("  [ERROR] Invalid ID: %s\n", status_message(status));
        return;
    }

    printf("  Task Name (letters, digits, underscore): ");
    read_line(buf, sizeof(buf));
    trimmed = trim_whitespace(buf);
    status = validate_task_name(trimmed);
    if (status != STATUS_OK) {
        printf("  [ERROR] %s\n", status_message(status));
        return;
    }
    snprintf(task.name, sizeof(task.name), "%s", trimmed);

    printf("  Period in ms (%d..%d): ", MIN_TIME_MS, MAX_PERIOD_MS);
    read_line(buf, sizeof(buf));
    status = parse_int_strict(trim_whitespace(buf), &task.period_ms);
    if (status != STATUS_OK) {
        printf("  [ERROR] Invalid period: %s\n", status_message(status));
        return;
    }

    printf("  WCET in ms (%d..%d): ", MIN_TIME_MS, task.period_ms);
    read_line(buf, sizeof(buf));
    status = parse_int_strict(trim_whitespace(buf), &task.exec_time_ms);
    if (status != STATUS_OK) {
        printf("  [ERROR] Invalid execution time: %s\n", status_message(status));
        return;
    }

    status = task_set_add(set, &task);
    if (status == STATUS_OK) {
        printf("  [OK] Task '%s' (ID=%d, T=%dms, C=%dms) added.\n",
               task.name, task.id, task.period_ms, task.exec_time_ms);
    } else {
        printf("  [ERROR] Could not add task: %s\n", status_message(status));
    }
}

static void handle_view_tasks(const TaskSet *set) {
    if (set->count <= 0) {
        printf("  [INFO] Task set is currently empty.\n");
        return;
    }
    printf("\n");
    write_task_table(stdout, set);
}

static void handle_save_csv(const TaskSet *set) {
    char buf[INPUT_BUF_LEN];
    char *path;
    StatusCode status;

    if (set->count <= 0) {
        printf("  [ERROR] Cannot save an empty task set.\n");
        return;
    }

    printf("  Enter output CSV path [default: %s]: ", DEFAULT_CSV_PATH);
    read_line(buf, sizeof(buf));
    path = trim_whitespace(buf);

    if (path[0] == '\0') {
        path = DEFAULT_CSV_PATH;
    }

    status = save_tasks_to_csv(path, set);
    if (status == STATUS_OK) {
        printf("  [OK] Saved %d task%s to '%s'.\n",
               set->count, set->count == 1 ? "" : "s", path);
    } else {
        printf("  [ERROR] Failed to save '%s': %s\n", path, status_message(status));
    }
}

static void prompt_save_report(const TaskSet *set, const SchedAnalysis *analysis,
                               const SimulationResult *result, const char *default_name) {
    char buf[INPUT_BUF_LEN];
    char *ans;
    StatusCode status;

    printf("  Save full report to '%s'? (y/N): ", default_name);
    read_line(buf, sizeof(buf));
    ans = trim_whitespace(buf);

    if (ans[0] == 'y' || ans[0] == 'Y') {
        status = save_report_to_file(default_name, set, analysis, result);
        if (status == STATUS_OK) {
            printf("  [OK] Report saved to '%s'.\n", default_name);
        } else {
            printf("  [ERROR] Failed to save report: %s\n", status_message(status));
        }
    }
}

static void handle_run_policy(const TaskSet *set, SchedPolicy policy) {
    SchedAnalysis analysis;
    SimulationResult result;
    StatusCode status;
    const char *report_file = (policy == POLICY_RMS) ? "rms_report.txt" : "edf_report.txt";

    if (set->count <= 0) {
        printf("  [ERROR] Load or add at least one task first.\n");
        return;
    }

    status = analyze_schedulability(set, policy, &analysis);
    if (status != STATUS_OK) {
        printf("  [ERROR] Analysis failed: %s\n", status_message(status));
        return;
    }

    status = simulate_schedule(set, policy, &result);
    if (status != STATUS_OK) {
        printf("  [ERROR] Simulation failed: %s\n", status_message(status));
        return;
    }

    printf("\n");
    write_full_report(stdout, set, &analysis, &result);
    prompt_save_report(set, &analysis, &result, report_file);

    free_simulation_result(&result);
}

static void handle_compare_policies(const TaskSet *set) {
    SchedAnalysis rms_analysis;
    SchedAnalysis edf_analysis;
    SimulationResult rms_result;
    SimulationResult edf_result;
    StatusCode status;

    if (set->count <= 0) {
        printf("  [ERROR] Load or add at least one task first.\n");
        return;
    }

    status = analyze_schedulability(set, POLICY_RMS, &rms_analysis);
    if (status != STATUS_OK) {
        printf("  [ERROR] RMS analysis failed: %s\n", status_message(status));
        return;
    }

    status = analyze_schedulability(set, POLICY_EDF, &edf_analysis);
    if (status != STATUS_OK) {
        printf("  [ERROR] EDF analysis failed: %s\n", status_message(status));
        return;
    }

    status = simulate_schedule(set, POLICY_RMS, &rms_result);
    if (status != STATUS_OK) {
        printf("  [ERROR] RMS simulation failed: %s\n", status_message(status));
        return;
    }

    status = simulate_schedule(set, POLICY_EDF, &edf_result);
    if (status != STATUS_OK) {
        free_simulation_result(&rms_result);
        printf("  [ERROR] EDF simulation failed: %s\n", status_message(status));
        return;
    }

    printf("\n=========================================================================\n");
    printf("  RMS vs EDF SIDE-BY-SIDE COMPARISON (Hyperperiod = %d ms)\n",
           rms_analysis.hyperperiod_ms);
    printf("=========================================================================\n");
    printf("  Metric                  | RMS                        | EDF\n");
    printf("  ------------------------+----------------------------+-------------------\n");
    printf("  CPU Utilization         | %6.2f%% (Bound: %6.2f%%)  | %6.2f%% (<=100%%)\n",
           rms_analysis.utilization * 100.0,
           rms_analysis.rms_bound * 100.0,
           edf_analysis.utilization * 100.0);
    printf("  Analytical Verdict      | %-26s | %s\n",
           verdict_name(rms_analysis.verdict),
           verdict_name(edf_analysis.verdict));
    printf("  Total Deadline Misses   | %-26d | %d\n",
           rms_result.total_deadline_misses,
           edf_result.total_deadline_misses);
    printf("  Context Switches        | %-26d | %d\n",
           rms_result.context_switches,
           edf_result.context_switches);
    printf("  Busy / Idle Time (ms)   | %d / %-22d | %d / %d\n",
           rms_result.busy_ms, rms_result.idle_ms,
           edf_result.busy_ms, edf_result.idle_ms);
    printf("=========================================================================\n\n");

    free_simulation_result(&rms_result);
    free_simulation_result(&edf_result);
}

int main(int argc, char *argv[]) {
    TaskSet set;
    char buf[INPUT_BUF_LEN];
    int choice;
    int error_line = 0;
    StatusCode status;

    status = task_set_init(&set);
    if (status != STATUS_OK) {
        fprintf(stderr, "Fatal error: %s\n", status_message(status));
        return 1;
    }

    print_banner();

    if (argc > 1) {
        status = load_tasks_from_csv(argv[1], &set, &error_line);
        if (status == STATUS_OK) {
            printf("  [OK] Auto-loaded %d task%s from '%s'.\n",
                   set.count, set.count == 1 ? "" : "s", argv[1]);
        } else if (error_line > 0) {
            printf("  [WARN] %s (Line %d): %s\n", argv[1], error_line, status_message(status));
        } else {
            printf("  [WARN] Could not auto-load '%s': %s\n", argv[1], status_message(status));
        }
    }

    for (;;) {
        print_menu(set.count);
        read_line(buf, sizeof(buf));

        if (feof(stdin)) {
            printf("\n");
            break;
        }

        if (parse_int_strict(trim_whitespace(buf), &choice) != STATUS_OK) {
            printf("  [ERROR] Please enter a valid menu number.\n");
            continue;
        }

        switch (choice) {
            case 1:
                handle_load_csv(&set);
                break;
            case 2:
                handle_add_task(&set);
                break;
            case 3:
                handle_view_tasks(&set);
                break;
            case 4:
                set.count = 0;
                printf("  [OK] Task set cleared.\n");
                break;
            case 5:
                handle_save_csv(&set);
                break;
            case 6:
                handle_run_policy(&set, POLICY_RMS);
                break;
            case 7:
                handle_run_policy(&set, POLICY_EDF);
                break;
            case 8:
                handle_compare_policies(&set);
                break;
            case 0:
                task_set_free(&set);
                printf("  Goodbye.\n");
                return 0;
            default:
                printf("  [ERROR] Unknown menu option: %d\n", choice);
                break;
        }
    }

    task_set_free(&set);
    return 0;
}