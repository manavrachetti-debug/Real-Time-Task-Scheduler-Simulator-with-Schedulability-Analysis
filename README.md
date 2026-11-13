# Real-Time Task Scheduler Simulator & Schedulability Analyzer

A modular C-based simulation engine for Rate Monotonic Scheduling (RMS) and Earliest Deadline First (EDF) real-time task sets, featuring a hand-written min-heap priority queue, CSV task ingestion, and automated CPU utilization analysis.

---

## Features
- **Schedulability Analysis:** Computes CPU utilization ($U$), the Liu-Layland RMS bound ($n(2^{1/n}-1)$), and exact hyperperiod LCM with overflow protection.
- **Priority Queue Engine:** Custom binary min-heap implementation for $O(\log n)$ job scheduling.
- **Dual-Policy Simulation:** Simulates time-stepped discrete execution for both **RMS** (Rate Monotonic) and **EDF** (Earliest Deadline First) policies.
- **Robust Ingestion & Reporting:** Strictly validates CSV input lines and task constraints, writing detailed summary reports and execution timelines to terminal or `.txt` log files.

---

## Build & Test Instructions

Using GNU Make:

```powershell
# Build the main simulator executable
mingw32-make

# Run the simulator
mingw32-make run

# Compile and execute the full unit test suite
mingw32-make test

# Clean compiled objects and logs
mingw32-make clean