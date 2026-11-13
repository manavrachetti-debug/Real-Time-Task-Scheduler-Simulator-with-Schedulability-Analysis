# System Architecture & Design Specification

## 1. Overview
The **Real-Time Task Scheduler Simulator & Schedulability Analyzer** is a modular C11 software engine designed to evaluate and simulate periodic real-time task sets under **Rate Monotonic Scheduling (RMS)** and **Earliest Deadline First (EDF)** policies.

---

## 2. Module Decomposition

| Module | Header | Responsibility |
| :--- | :--- | :--- |
| **Validation** | `include/validation.h` | Enforces strict input bounds (`MAX_TASKS = 20`, `MAX_PERIOD_MS = 1000`, `MAX_HYPERPERIOD_MS = 10000`), validates task identifiers/names, and performs safe integer parsing. |
| **File Handler** | `include/file_handler.h` | Manages dynamic `TaskSet` allocation (`malloc`/`realloc`) and parses/exports CSV task definitions with UTF-8 BOM handling and line-level error reporting. |
| **Priority Queue** | `include/priority_queue.h` | Implements a generic dynamic binary min-heap (`O(log n)` push/pop) parameterized by a `JobCompare` function pointer. |
| **Analysis** | `include/analysis.h` | Computes exact CPU utilization ($U = \sum C_i / T_i$), Liu-Layland RMS bound ($n(2^{1/n}-1)$), overflow-guarded hyperperiod LCM, and analytical schedulability verdicts. |
| **Scheduler** | `include/scheduler.h` | Executes a discrete 1 ms tick simulation over `[0, H)` with job releases, preemption tracking, context-switch counting, and deadline-miss detection. |
| **Report** | `include/report.h` | Formats ASCII task tables, 50-column execution timelines, and per-task response-time statistics to `stdout` or persistent `.txt` reports. |

---

## 3. Core Data Structures & Algorithms

### 3.1 Binary Min-Heap (`PriorityQueue`)
Jobs are stored in a contiguous heap-allocated array representing a complete binary tree:
* **Parent index:** $(i - 1) / 2$
* **Left / Right child indices:** $2i + 1$ and $2i + 2$
* **Ordering invariant:** `compare(&items[parent], &items[child]) <= 0`
* **Tie-breaking:** Both `compare_rms` (by `period_ms`) and `compare_edf` (by `absolute_deadline`) break ties deterministically by `release_time`, then `task_index`.

### 3.2 Discrete-Time Simulation Loop
For each millisecond $t \in [0, H)$:
1. **Release Phase:** Every task $i$ where $t \bmod T_i = 0$ releases a new `Job` with `absolute_deadline = t + T_i` and `remaining_time = C_i`.
2. **Dispatch Phase:** If the ready queue is empty, record `IDLE_TASK_INDEX` (`-1`). Otherwise, pop the highest-priority job, increment `context_switches` if the active task changed from the previous tick, and decrement `remaining_time`.
3. **Completion & Miss Phase:** When `remaining_time == 0`, record `response_ms = (t + 1) - release_time` and flag a deadline miss if `t + 1 > absolute_deadline`. Any jobs still remaining in the queue at $t = H$ are counted as deadline misses.

---

## 4. Error Handling Strategy
Every fallible function returns a uniform `StatusCode` enum (`STATUS_OK = 0`, `ERR_NULL_POINTER`, `ERR_INVALID_PERIOD`, `ERR_HYPERPERIOD_TOO_LARGE`, `ERR_MEMORY`, etc.), guaranteeing zero silent failures, safe cleanup on allocation errors, and deterministic unit testing.