#include "../include/analysis.h"
#include <math.h>

double calculate_utilization(const Task tasks[], int count) {
    double total_utilization = 0.0;
    for (int i = 0; i < count; i++) {
        total_utilization += (double)tasks[i].execution_time / tasks[i].period;
    }
    return total_utilization;
}

// Helper function to find Greatest Common Divisor (GCD)
static int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Helper function to find Least Common Multiple (LCM)
static int lcm(int a, int b) {
    return (a / gcd(a, b)) * b;
}

int calculate_hyperperiod(const Task tasks[], int count) {
    if (count <= 0) return 0;
    int hp = tasks[0].period;
    for (int i = 1; i < count; i++) {
        hp = lcm(hp, tasks[i].period);
    }
    return hp;
}

int check_rms_bound(double utilization, int task_count) {
    // Liu & Layland bound: U <= n * (2^(1/n) - 1)
    double bound = task_count * (pow(2.0, 1.0 / task_count) - 1.0);
    return (utilization <= bound);
}