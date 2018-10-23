#include "pqueue.h"


#include <math.h>
#include <stdio.h>
#include <string.h>

struct _item
{
    void           *data;
    double          priority;
};

struct _pqueue
{
    struct _item   *data;
    int             (
    *cmp)           (
    void *,
    void *);
    size_t          size;
    size_t          capacity;
};

static void
swap(
    struct _item *a,
    struct _item *b)
{
    struct _item    tmp = *a;

    *a = *b;
    *b = tmp;
}

// "Bubbles down" a value through the heap
static void
bubble_down(
    struct _pqueue *pq,
    size_t idx)
{
    while (2 * idx + 1 < pq->size)
    {
        size_t          left = 2 * idx + 1;
        size_t          right = 2 * idx + 2;
        size_t          to_swap = idx;

        if (pq->data[left].priority < pq->data[to_swap].priority)
        {
            to_swap = left;
        }
        if (right < pq->size &&
            pq->data[right].priority < pq->data[to_swap].priority)
        {
            to_swap = right;
        }

        if (to_swap == idx)
        {
            break;
        }

        swap(&pq->data[idx], &pq->data[to_swap]);

        idx = to_swap;
    }
}

static void
bubble_up(
    pqueue * pq,
    size_t idx)
{
    size_t          parent_idx = (idx - 1) / 2;

    while (idx > 0)
    {
        // Exit the loop if the heap property is satisfied
        if (pq->data[parent_idx].priority <= pq->data[idx].priority)
        {
            break;
        }
        swap(&pq->data[parent_idx], &pq->data[idx]);

        // Move up the heap
        idx = parent_idx;
        parent_idx = (idx - 1) / 2;
    }
}

pqueue         *
pqueue_create(
    int             (*cmp) (void *,
                            void *))
{
    if (!cmp)
    {
        return NULL;
    }

    struct _pqueue *pq = malloc(sizeof(*pq));

    if (!pq)
    {
        return NULL;
    }

    pq->size = 0;
    pq->capacity = 4;
    pq->cmp = cmp;
    pq->data = malloc(pq->capacity * sizeof(*pq->data));
    if (!pq->data)
    {
        free(pq);
        return NULL;
    }

    return pq;
}

size_t
pqueue_size(
    const pqueue * pq)
{
    if (!pq)
    {
        return 0;
    }
    return pq->size;
}

bool
pqueue_enqueue(
    pqueue * pq,
    void *item,
    double priority)
{
    if (!pq)
    {
        return false;
    }

    // Double the capacity if needed
    if (pq->size == pq->capacity)
    {
        void           *tmp = realloc(pq->data,
                                      2 * pq->capacity * sizeof(*pq->data));

        if (!tmp)
        {
            return false;
        }
        pq->capacity *= 2;
        pq->data = tmp;
    }

    size_t          current_idx = pq->size;

    pq->size++;
    pq->data[current_idx].data = item;
    pq->data[current_idx].priority = priority;

    bubble_up(pq, current_idx);

    return true;
}

bool
pqueue_reprioritize(
    pqueue * pq,
    void *item,
    double priority)
{
    if (!pq || !item)
    {
        return false;
    }

    for (size_t n = 0; n < pq->size; ++n)
    {
        if (pq->cmp(pq->data[n].data, item) == 0)
        {
            double          old_priority = pq->data[n].priority;

            pq->data[n].priority = priority;
            if (old_priority < priority)
            {
                bubble_down(pq, n);
            }
            else
            {
                bubble_up(pq, n);
            }
        }
    }

    return NULL;
}

double
pqueue_dequeue(
    pqueue * pq,
    void **item)
{
    if (!pq || pq->size == 0)
    {
        *item = NULL;
        return NAN;
    }

    double          priority = pq->data[0].priority;

    *item = pq->data[0].data;
    pq->size--;

    pq->data[0] = pq->data[pq->size];

    bubble_down(pq, 0);

    return priority;
}

void           *
pqueue_search(
    const pqueue * pq,
    void *item)
{
    if (!pq)
    {
        return NULL;
    }
    for (size_t n = 0; n < pq->size; ++n)
    {
        if (pq->cmp(pq->data[n].data, item) == 0)
        {
            return pq->data[n].data;
        }
    }

    return NULL;
}

void
pqueue_destroy(
    pqueue * pq)
{
    free(pq->data);
    free(pq);
}


void
heap_print(
    const pqueue * pq)
{
    if (!pq)
    {
        return;
    }

    printf("SZ: %zu  CAP: %zu\n", pq->size, pq->capacity);
    for (size_t n = 0; n < pq->size; ++n)
    {
        printf("%zu: %lf: %p\n", n, pq->data[n].priority, pq->data[n].data);
    }
}
