#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <unistd.h>

#include "../graph/Graph.h"

/**
 * @brief Find path between start and end
 * @param g Graph to traverse
 * @param start starting node in graph
 * @param end ending node in graph
 * @param path output parameter which is filled with shortest start-to-end path
 * @return number of nodes in path (negative if error or disconnected)
 */
ssize_t         Dijkstra_path(
    const Graph * g,
    const char *start,
    const char *end,
    char ***path);

/**
 * @brief Changes 2d array of the maze and changes the route to show the solved maze
 * @param mazeFromFile 2d array of maze
 * @param route Nodes in graph that lead to end
 * @param hops Number of hops for tracking in loop
 */
void            Dijkstra_solveMaze(
    char **mazeFromFile,
    char **route,
    size_t hops);

#endif
