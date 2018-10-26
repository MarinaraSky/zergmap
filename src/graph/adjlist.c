#define _XOPEN_SOURCE 500
#include "Graph.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct edge_
{
    double          weight;
    struct node_   *to;
    struct edge_   *next;
} edge_;

typedef struct node_
{
    char           *name;
    struct node_   *next;
    struct edge_   *edges;
} node_;

struct graph
{
    struct node_   *root;
};

static struct node_ *
getNode(
    const Graph * g,
    const char *name)
{
    struct node_   *curr = g->root;

    while (curr)
    {
        if (strcmp(name, curr->name) == 0)
        {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

static void
removeNode(
    struct node_ *curr)
{
    struct edge_   *e = curr->edges;

    while (e)
    {
        struct edge_   *tmp = e->next;

        free(e);
        e = tmp;
    }

    free(curr->name);
    free(curr);
}

Graph          *
Graph_create(
    void)
{
    Graph          *g = malloc(sizeof(*g));

    if (!g)
    {
        return NULL;
    }

    g->root = NULL;

    return g;
}

bool
Graph_addNode(
    Graph * g,
    const char *name)
{
    if (!g || !name)
    {
        return false;
    }

    // Return false if node already exists
    if (getNode(g, name))
    {
        return false;
    }

    struct node_   *created = malloc(sizeof(*created));

    if (!created)
    {
        return false;
    }

    created->name = strdup(name);
    if (!created->name)
    {
        free(created);
        return false;
    }
    created->edges = NULL;
    created->next = g->root;

    g->root = created;

    return true;
}

bool
Graph_addEdge(
    Graph * g,
    const char *from,
    const char *to,
    double weight)
{
    if (!g || !from || !to || isnan(weight))
    {
        return false;
    }

    struct node_   *curr = getNode(g, from);

    if (!curr)
    {
        return false;
    }

    struct edge_   *e = curr->edges;

    while (e)
    {
        // If edge already exists, skip it
        if (strcmp(to, e->to->name) == 0)
        {
            return false;
        }
        e = e->next;
    }

    // Ready to add new edge
    struct edge_   *created = malloc(sizeof(*created));

    if (!created)
    {
        return false;
    }
    created->weight = weight;
    created->next = curr->edges;
    created->to = getNode(g, to);
    if (!created->to)
    {
        free(created);
        return false;
    }
    curr->edges = created;
    return true;

}

bool
Graph_isAdjacent(
    const Graph * g,
    const char *from,
    const char *to)
{
    return !isnan(Graph_getEdgeWeight(g, from, to));
}

ssize_t
Graph_getNodes(
    const Graph * g,
    char ***nodes)
{
    if (!g || !nodes)
    {
        errno = EINVAL;
        return -1;
    }

    ssize_t         nodeCount = 0;
    struct node_   *curr = g->root;

    while (curr)
    {
        ++nodeCount;
        curr = curr->next;
    }

    char          **results = malloc(nodeCount * sizeof(*results));

    if (!results)
    {
        errno = ENOMEM;
        return -1;
    }

    nodeCount = 0;
    curr = g->root;
    while (curr)
    {
        results[nodeCount++] = curr->name;
        curr = curr->next;
    }

    *nodes = results;
    return nodeCount;
}

ssize_t
Graph_getNeighbors(
    const Graph * g,
    const char *name,
    char ***neighbors)
{
    if (!g || !name || !neighbors)
    {
        errno = EINVAL;
        return -1;
    }

    struct node_   *curr = getNode(g, name);

    if (!curr)
    {
        errno = ENOENT;
        return -1;
    }

    if (!curr->edges)
    {
        *neighbors = NULL;
        return 0;
    }

    ssize_t         edgeCount = 0;
    struct edge_   *e = curr->edges;

    while (e)
    {
        ++edgeCount;
        e = e->next;
    }

    char          **results = malloc(edgeCount * sizeof(*results));

    if (!results)
    {
        errno = ENOMEM;
        return -1;
    }

    edgeCount = 0;
    e = curr->edges;
    while (e)
    {
        results[edgeCount] = e->to->name;
        ++edgeCount;
        e = e->next;
    }

    *neighbors = results;
    return edgeCount;
}

double
Graph_getEdgeWeight(
    const Graph * g,
    const char *from,
    const char *to)
{
    if (!g || !from || !to)
    {
        errno = EINVAL;
        return NAN;
    }

    struct node_   *curr = getNode(g, from);

    if (!curr)
    {
        errno = EINVAL;
        return NAN;
    }

    struct edge_   *e = curr->edges;

    while (e)
    {
        if (strcmp(e->to->name, to) == 0)
        {
            return e->weight;
        }

        e = e->next;
    }

    return NAN;
}

bool
Graph_updateEdgeWeight(
    const Graph * g,
    const char *from,
    const char *to,
	double weight)
{
    if (!g || !from || !to)
    {
        errno = EINVAL;
        return true;
    }

    struct node_   *curr = getNode(g, from);

    if (!curr)
    {
        errno = EINVAL;
        return false;
    }

    struct edge_   *e = curr->edges;

    while (e)
    {
        if (strcmp(e->to->name, to) == 0)
        {
            e->weight = weight;
        }

        e = e->next;
    }

    return true;
}

void
Graph_deleteNode(
    Graph * g,
    const char *name)
{
    if (!g || !name)
    {
        return;
    }

    struct node_   *curr = g->root;

    if (!curr)
    {
        return;
    }

    while (curr)
    {
        if (strcmp(curr->name, name) == 0)
        {
            curr = curr->next;
            continue;
        }

        Graph_deleteEdge(g, curr->name, name);
        curr = curr->next;
    }

    curr = g->root;
    if (strcmp(curr->name, name) == 0)
    {
        g->root = curr->next;
        removeNode(curr);
        return;
    }

    struct node_   *nextNode = curr->next;

    while (nextNode)
    {
        if (strcmp(nextNode->name, name) == 0)
        {
            curr->next = nextNode->next;
            removeNode(nextNode);
            return;
        }

        curr = curr->next;
        nextNode = nextNode->next;
    }
}

void
Graph_deleteEdge(
    Graph * g,
    const char *from,
    const char *to)
{
    if (!g || !from || !to)
    {
        return;
    }

    struct node_   *curr = getNode(g, from);

    if (!curr)
    {
        return;
    }

    struct edge_   *e = curr->edges;

    if (!e)
    {
        return;
    }

    struct edge_   *nextEdge = e->next;

    if (strcmp(e->to->name, to) == 0)
    {
        curr->edges = nextEdge;
        free(e);
        return;
    }

    while (nextEdge)
    {
        if (strcmp(nextEdge->to->name, to) == 0)
        {
            e->next = nextEdge->next;
            free(nextEdge);
            return;
        }

        e = nextEdge;
        nextEdge = nextEdge->next;
    }
}

void
Graph_print(
    const Graph * g)
{
    if (!g)
    {
        return;
    }

    struct node_   *curr = g->root;

    while (curr)
    {
        printf("%s: ", curr->name);

        struct edge_   *e = curr->edges;

        while (e)
        {
            printf("→ %s ", e->to->name);
            e = e->next;
        }
        puts("");

        curr = curr->next;
    }
}

void
Graph_disassemble(
    Graph * g)
{
    if (!g)
    {
        return;
    }

    struct node_   *curr = g->root;

    while (curr)
    {
        struct edge_   *e = curr->edges;

        while (e)
        {
            struct edge_   *tmp = e->next;

            free(e);
            e = tmp;
        }

        struct node_   *tmpNode = curr->next;

        free(curr->name);
        free(curr);
        curr = tmpNode;
    }

    free(g);
}
