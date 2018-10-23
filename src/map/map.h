#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

// Uses strings for keys and doubles for values
// key may not be NULL

typedef struct _map map;


/**
 * @brief Creates an empty map structure
 * @return Pointer to new map in memory
 */
map            *map_create(
    void);

/**
 * @brief Inserts new key and value into map
 * @param m Map to insert into
 * @param key Key value to add to map
 * @param value Value for specified key
 * @return True if added successfully, False if failed
 */
bool            map_insert(
    map * m,
    const char *key,
    double value);

/**
 * @brief Checks if key exists in map
 * @param m Map to inspect
 * @param key Key value to look for in map
 * @return True if found successfully, False if not found
 */
bool            map_exists(
    map * m,
    const char *key);

/**
 * @brief Returns Value of specified key
 * @param m Map to inspect
 * @param key Key value to look for in map
 * @return Value at key specified
 */
double          map_lookup(
    map * m,
    const char *key);

/**
 * @brief Breaks down map and frees memory
 * @param m Map to destroy
 */
void            map_destroy(
    map * m);

#endif
