#define _XOPEN_SOURCE 500
#include "vmap.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/md5.h>

struct entry
{
    char           *key;
    void           *value;
    struct entry   *next;
};

struct _vmap
{
    struct entry   *data;
    size_t          size;
    size_t          capacity;
};

static uint64_t
hash(
    const unsigned char *str)
{
    // This would normally be MD5_DIGEST_LENGTH in uint8_t's, so it translates
    // to 2 uint64_t's.
    uint64_t        buf[2];

    MD5(str, strlen((const char *) str), (unsigned char *) buf);

    // Either half is acceptable
    return buf[1];
}

static void
ll_destroy(
    struct entry *e)
{
    while (e)
    {
        struct entry   *next = e->next;

        free(e->key);
        free(e);
        e = next;
    }
}

vmap           *
vmap_create(
    void)
{
    vmap           *m = malloc(sizeof(*m));

    if (!m)
    {
        return NULL;
    }

    m->size = 0;
    m->capacity = 16;
    m->data = calloc(m->capacity, sizeof(*m->data));
    if (!m->data)
    {
        free(m);
        return NULL;
    }

    return m;
}

bool
vmap_insert(
    vmap * m,
    const char *key,
    void *value)
{
    if (!m || !key)
    {
        return false;
    }

    if (m->size >= m->capacity / 3)
    {
        struct entry   *tmp = calloc(2 * m->capacity, sizeof(*tmp));

        if (!tmp)
        {
            return false;
        }
        struct entry   *old = m->data;

        m->capacity *= 2;
        m->size = 0;
        m->data = tmp;

        for (size_t n = 0; n < m->capacity / 2; ++n)
        {
            struct entry   *target = &old[n];

            while (target->key)
            {
                vmap_insert(m, target->key, target->value);
                target = target->next;
            }
            free(old[n].key);
            ll_destroy(old[n].next);
        }
        free(old);
    }

    uint64_t        idx = hash((unsigned char *) key) % m->capacity;

    struct entry   *target = &m->data[idx];

    while (target->next)
    {
        if (strcmp(target->key, key) == 0)
        {
            target->value = value;
            return true;
        }

        target = target->next;
    }

    target->key = strdup(key);
    target->value = value;
    target->next = calloc(1, sizeof(*target->next));

    m->size++;

    return true;
}

bool
vmap_exists(
    vmap * m,
    const char *key)
{
    if (!m || !key)
    {
        return false;
    }

    uint64_t        idx = hash((unsigned char *) key) % m->capacity;

    struct entry   *target = &m->data[idx];

    while (target && target->key)
    {
        if (strcmp(target->key, key) == 0)
        {
            return true;
        }

        target = target->next;
    }

    return false;
}

void           *
vmap_lookup(
    vmap * m,
    const char *key)
{
    if (!m || !key)
    {
        return false;
    }

    uint64_t        idx = hash((unsigned char *) key) % m->capacity;
    struct entry   *target = &m->data[idx];

    while (target && target->key)
    {
        if (strcmp(target->key, key) == 0)
        {
            return target->value;
        }

        target = target->next;
    }

    return NULL;
}

void
vmap_destroy(
    vmap * m)
{
    if (!m)
    {
        return;
    }

    for (size_t n = 0; n < m->capacity; ++n)
    {
        if (m->data[n].key)
        {
            free(m->data[n].key);
            ll_destroy(m->data[n].next);
        }
    }
    free(m->data);
    free(m);
}
