#include "../include/priority_queue.h"
#include <stdio.h>

void heap_init(MinHeap *hp, SchedPolicy policy) {
    hp->size = 0;
    hp->policy = policy;
}

int is_higher_priority(Job j1, Job j2, SchedPolicy policy) {
    if (policy == SCHED_EDF) {
        if (j1.absolute_deadline != j2.absolute_deadline) {
            return j1.absolute_deadline < j2.absolute_deadline;
        }
    }
    return j1.task_id < j2.task_id;
}

static void swap(Job *a, Job *b) {
    Job temp = *a;
    *a = *b;
    *b = temp;
}

int heap_push(MinHeap *hp, Job job) {
    if (hp->size >= MAX_JOBS) {
        return 0; // Overflow
    }
    
    int curr = hp->size;
    hp->data[curr] = job;
    hp->size++;

    while (curr > 0) {
        int parent = (curr - 1) / 2;
        if (is_higher_priority(hp->data[curr], hp->data[parent], hp->policy)) {
            swap(&hp->data[curr], &hp->data[parent]);
            curr = parent;
        } else {
            break;
        }
    }
    return 1;
}

Job heap_pop(MinHeap *hp) {
    Job empty = {-1, -1, -1, -1};
    if (hp->size <= 0) {
        return empty; // Underflow
    }

    Job root = hp->data[0];
    hp->size--;
    hp->data[0] = hp->data[hp->size];

    int curr = 0;
    while ((2 * curr + 1) < hp->size) {
        int left = 2 * curr + 1;
        int right = left + 1;
        int target = left;

        if (right < hp->size && is_higher_priority(hp->data[right], hp->data[left], hp->policy)) {
            target = right;
        }

        if (is_higher_priority(hp->data[target], hp->data[curr], hp->policy)) {
            swap(&hp->data[target], &hp->data[curr]);
            curr = target;
        } else {
            break;
        }
    }

    return root;
}