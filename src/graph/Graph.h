#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <unistd.h>

typedef struct graph Graph;

/**
 * @brief Create an empty graph structure
 * @return the graph structure, or NULL on error
 */
Graph          *Graph_create(
    void);


/**
 * @brief Adds a node to the graph (does not add
 * duplicates)
 * @param g Graph to modify
 * @param name Name of new node
 * @return true for successful add
 */
bool            Graph_addNode(
    Graph * g,
    const char *name);

/**
 * @brief Adds an edge to the graph (does not add
 * duplicates)
 * @param g Graph to modify
 * @param from Name of source node
 * @param to Name of destination node
 * @param weight Cost of the edge
 * @return true for successful add
 */
bool            Graph_addEdge(
    Graph * g,
    const char *from,
    const char *to,
    double weight);

/**
 * @brief Checks if two nodes are adjacent
 * @param g Graph to inspect
 * @param from Name of source node
 * @param to Name of destination node
 * @return True if nodes are adjacent, false otherwise
 */
bool            Graph_isAdjacent(
    const Graph * g,
    const char *from,
    const char *to);

/**
 * @brief provide list of nodes of a graph
 * @param g Graph to inspect
 * @param nodes input parameter to store array of nodes' names
 * @return number of nodes found (-1 for error)
 */
ssize_t         Graph_getNodes(
    const Graph * g,
    char ***nodes);

/**
 * @brief provide list of neighbor's names for a given node
 * @param g Graph to inspect
 * @param name node's name to find neighbors for
 * @param neighbors input parameter to store array of neighbors' names
 * @return number of neighbors found (-1 for error)
 */
ssize_t         Graph_getNeighbors(
    const Graph * g,
    const char *name,
    char ***neighbors);

/**
 * @brief Provide edge weight between two nodes
 * @param g Graph to inspect
 * @param from Starting node
 * @param to neighbor node
 * @return weight of edge (NAN if edge does not exist)
 */
double          Graph_getEdgeWeight(
    const Graph * g,
    const char *from,
    const char *to);

/**
 * @brief Update edge weight between two nodes
 * @param g Graph to inspect
 * @param from Starting node
 * @param to neighbor node
 * @param weight new weight of edge
 * @return True if successfuly updated, False if failed to update
 */
bool
Graph_updateEdgeWeight(
    const Graph * g,
    const char *from,
    const char *to,
	double weight);
/**
 * @brief Remove a node from the graph
 * @param g Graph to alter
 * @param name Name of node to remove
 */
void            Graph_deleteNode(
    Graph * g,
    const char *name);

/**
 * @brief Remove an edge from the graph
 * @param g Graph to alter
 * @param from Starting node of edge
 * @param to Ending node of edge
 */
void            Graph_deleteEdge(
    Graph * g,
    const char *from,
    const char *to);

/**
 * @brief Prints graph to stdout
 * @param g Graph to print
 */
void            Graph_print(
    const Graph * g);

/**
 * @brief Destroy the graph scaffolding without affecting the
 * underlying data
 *
 * @param g Graph to disassemble
 */
void            Graph_disassemble(
    Graph * g);


#endif
