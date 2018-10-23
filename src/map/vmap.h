#ifndef VMAP_H
#define VMAP_H

#include <stdbool.h>

// key may not be NULL

typedef struct _vmap vmap;


/**
 * @brief Creates an empty vmap structure
 * @return Pointer to new vmap in memory
 */
vmap           *vmap_create(
    void);

/**
 * @brief Inserts new key and value into vmap
 * @param m VMap to insert into
 * @param key Key value to add to vmap
 * @param value Value for specified key
 * @return True if added successfully, False if failed
 */
bool            vmap_insert(
    vmap * m,
    const char *key,
    void *value);

/**
 * @brief Checks if key exists in vmap
 * @param m VMap to inspect
 * @param key Key value to look for in vmap
 * @return True if found successfully, False if not found
 */
bool            vmap_exists(
    vmap * m,
    const char *key);

/**
 * @brief Returns Value of specified key
 * @param m VMap to inspect
 * @param key Key value to look for in vmap
 * @return Value at key specified
 */
void           *vmap_lookup(
    vmap * m,
    const char *key);

/**
 * @brief Breaks down vmap and frees memory
 * @param m VMap to destroy
 */
void            vmap_destroy(
    vmap * m);

#endif
