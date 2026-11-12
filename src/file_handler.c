#include "../include/file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_tasks_from_csv(const char *filename, Task tasks[], int max_tasks) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Could not open input file %s\n", filename);
        return -1;
    }

    char line[128];
    int count = 0;

    // Read header line if present
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return 0;
    }

    // Parse line by line: ID, Period, ExecutionTime, Deadline
    while (fgets(line, sizeof(line), fp) && count < max_tasks) {
        // Skip empty or comment lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        Task t;
        if (sscanf(line, "%d,%d,%d,%d", &t.id, &t.period, &t.execution_time, &t.deadline) == 4) {
            tasks[count++] = t;
        }
    }

    fclose(fp);
    return count;
}