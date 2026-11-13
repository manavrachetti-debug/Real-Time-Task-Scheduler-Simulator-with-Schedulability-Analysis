# Project Proposal: Real-Time Task Scheduler Simulator & Schedulability Analyzer

## 1. Problem Statement
In hard real-time embedded systems (such as automotive controllers, avionics, and industrial sensor loops), missing a task's periodic deadline can lead to system failure. Verifying whether a set of periodic tasks can meet all deadlines requires both mathematical schedulability tests and cycle-accurate execution simulation.

## 2. Proposed Solution
This project implements a standalone C11 command-line simulator and analytical verifier supporting two canonical uniprocessor real-time scheduling algorithms:
* **Rate Monotonic Scheduling (RMS):** Static priority assignment inversely proportional to task period ($T_i$), verified against the Liu & Layland utilization bound $U \le n(2^{1/n} - 1)$.
* **Earliest Deadline First (EDF):** Dynamic priority assignment based on the nearest absolute deadline ($d_k = r_k + T_i$), verified against the exact processor utilization threshold $U \le 1.0$.

## 3. Core Functional Scope
1. **Strict Input & CSV Ingestion:** Load task sets from structured CSV files or interactive console prompts with full boundary validation (`MAX_TASKS = 20`, `MAX_PERIOD_MS = 1000`, `MAX_HYPERPERIOD_MS = 10000`).
2. **Dynamic Binary Min-Heap:** Custom $O(\log n)$ priority queue supporting pluggable comparator functions (`compare_rms` and `compare_edf`).
3. **Hyperperiod Discrete Simulation:** Simulates every 1 ms tick across $[0, \text{LCM}(T_1, \dots, T_n))$, tracking preemptions, context switches, response times, and deadline misses.
4. **Automated Reporting & Side-by-Side Comparison:** Generates ASCII execution timelines, per-task statistics tables, and persistent `.txt` reports.

## 4. Verification Plan
Every core module (`validation`, `file_handler`, `priority_queue`, `analysis`, `scheduler`, `report`) is verified through an automated C unit test suite (`tests/test_runner.c`) testing positive cases, boundary conditions, and error handling paths.