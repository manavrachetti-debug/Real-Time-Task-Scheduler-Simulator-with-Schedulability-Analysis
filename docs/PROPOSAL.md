# Project Proposal: Real-Time Task Scheduler Simulator with Schedulability Analysis

## 1. Problem Statement & Scenario
In embedded and real-time systems (such as automotive control units or industrial sensor nodes), tasks must execute within strict time constraints. Missing a deadline can cause system instability or hardware failure. 

This project implements a discrete-time Real-Time Operating System (RTOS) task scheduler simulator in C to evaluate task set schedulability under two primary scheduling algorithms:
- **Rate Monotonic Scheduling (RMS):** Static priority policy based on task periods.
- **Earliest Deadline First (EDF):** Dynamic priority policy based on absolute deadlines.

## 2. Functional Requirements (FR)
- **FR-01:** Read task configurations (Task ID, Period, Execution Time, Relative Deadline) from a CSV file.
- **FR-02:** Validate input parameters (ensure $C_i \le T_i$, positive values, valid period ratios).
- **FR-03:** Implement a hand-written Min-Heap priority queue to manage active job instances efficiently.
- **FR-04:** Simulate task execution over a defined hyperperiod ($H = \text{LCM}(T_1, T_2, \dots, T_n)$).
- **FR-05:** Detect and log deadline misses during execution.
- **FR-06:** Compute total CPU utilization ($U = \sum \frac{C_i}{T_i}$) and compare RMS against the Liu & Layland bound ($U_{bound} = n(2^{1/n} - 1)$).
- **FR-07:** Export execution timeline logs and summary statistics to an output text file.

## 3. Architecture & Module Structure
The simulator consists of modular C units:
1. `validation.c / .h`: Validates CSV input and mathematical feasibility constraints.
2. `file_handler.c / .h`: Parses input CSV files and formats report output.
3. `priority_queue.c / .h`: Hand-written Min-Heap data structure for job queueing.
4. `scheduler.c / .h`: Core simulation loop handling time increments, job releases, and context switching.
5. `analysis.c / .h`: Computes CPU utilization, hyperperiods, and schedulability bounds.
6. `report.c / .h`: Generates Gantt-style text timelines and metrics summaries.
7. `main.c`: CLI entry point for running simulations.