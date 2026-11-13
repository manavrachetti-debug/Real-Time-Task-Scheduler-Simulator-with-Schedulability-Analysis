#include <limits.h>
#include <stdlib.h>
#include "priority_queue.h"

static void swap_jobs(Job *a, Job *b)
{
    Job temp = *a;
    *a = *b;
    *b = temp;
}

static void sift_up(PriorityQueue *pq, int index)
{
    int parent;

    while (index > 0) {
        parent = (index - 1) / 2;
        if (pq->compare(&pq->items[index], &pq->items[parent]) < 0) {
            swap_jobs(&pq->items[index], &pq->items[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

static void sift_down(PriorityQueue *pq, int index)
{
    int left;
    int right;
    int best;

    for (;;) {
        left = 2 * index + 1;
        right = left + 1;
        best = index;

        if (left < pq->size &&
            pq->compare(&pq->items[left], &pq->items[best]) < 0) {
            best = left;
        }

        if (right < pq->size &&
            pq->compare(&pq->items[right], &pq->items[best]) < 0) {
            best = right;
        }

        if (best == index) {
            break;
        }

        swap_jobs(&pq->items[index], &pq->items[best]);
        index = best;
    }
}

StatusCode pq_init(PriorityQueue *pq, int initial_capacity, JobCompare compare)
{
    if (pq == NULL || compare == NULL) {
        return ERR_NULL_POINTER;
    }

    pq->items = NULL;
    pq->size = 0;
    pq->capacity = 0;
    pq->compare = NULL;

    if (initial_capacity < 1) {
        return ERR_INVALID_NUMBER;
    }

    pq->items = (Job *)malloc((size_t)initial_capacity * sizeof(Job));
    if (pq->items == NULL) {
        return ERR_MEMORY;
    }

    pq->capacity = initial_capacity;
    pq->compare = compare;
    return STATUS_OK;
}

StatusCode pq_push(PriorityQueue *pq, const Job *job)
{
    Job *grown;
    int new_capacity;

    if (pq == NULL || job == NULL || pq->items == NULL ||
        pq->compare == NULL) {
        return ERR_NULL_POINTER;
    }

    if (pq->size >= pq->capacity) {
        if (pq->capacity > INT_MAX / 2) {
            return ERR_MEMORY;
        }

        new_capacity = pq->capacity * 2;
        grown = (Job *)realloc(pq->items, (size_t)new_capacity * sizeof(Job));
        if (grown == NULL) {
            return ERR_MEMORY;
        }

        pq->items = grown;
        pq->capacity = new_capacity;
    }

    pq->items[pq->size] = *job;
    pq->size++;
    sift_up(pq, pq->size - 1);
    return STATUS_OK;
}

StatusCode pq_pop(PriorityQueue *pq, Job *out)
{
    if (pq == NULL || out == NULL) {
        return ERR_NULL_POINTER;
    }

    if (pq->size <= 0) {
        return ERR_HEAP_EMPTY;
    }

    if (pq->items == NULL || pq->compare == NULL) {
        return ERR_NULL_POINTER;
    }

    *out = pq->items[0];
    pq->size--;

    if (pq->size > 0) {
        pq->items[0] = pq->items[pq->size];
        sift_down(pq, 0);
    }

    return STATUS_OK;
}

StatusCode pq_peek(const PriorityQueue *pq, Job *out)
{
    if (pq == NULL || out == NULL) {
        return ERR_NULL_POINTER;
    }

    if (pq->size <= 0) {
        return ERR_HEAP_EMPTY;
    }

    if (pq->items == NULL) {
        return ERR_NULL_POINTER;
    }

    *out = pq->items[0];
    return STATUS_OK;
}

int pq_is_empty(const PriorityQueue *pq)
{
    if (pq == NULL) {
        return 1;
    }

    return pq->size <= 0;
}

int pq_size(const PriorityQueue *pq)
{
    if (pq == NULL) {
        return 0;
    }

    return pq->size;
}

void pq_free(PriorityQueue *pq)
{
    if (pq == NULL) {
        return;
    }

    free(pq->items);
    pq->items = NULL;
    pq->size = 0;
    pq->capacity = 0;
    pq->compare = NULL;
}