#ifndef PQUEUE_H
#define PQUEUE_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct _pqueue pqueue;


pqueue         *pqueue_create(
    int             (*cmp) (void *,
                            void *));

size_t          pqueue_size(
    const pqueue * pq);

bool            pqueue_enqueue(
    pqueue * pq,
    void *item,
    double priority);

bool            pqueue_reprioritize(
    pqueue * pq,
    void *item,
    double priority);

double          pqueue_dequeue(
    pqueue * pq,
    void **item);

void           *pqueue_search(
    const pqueue * pq,
    void *item);

void            pqueue_destroy(
    pqueue * pq);

#endif
