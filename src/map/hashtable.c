#define _XOPEN_SOURCE 500
#include "map.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/md5.h>

struct entry
{
    char           *key;
    double          value;
    struct entry   *next;
};

struct _map
{
    struct entry   *data;
    size_t          size;
    size_t          capacity;
};

static          uint64_t
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

map            *
map_create(
    void)
{
    map            *m = malloc(sizeof(*m));

    if (!m)
    {
        return NULL;
    }

    m->size = 0;
    m->capacity = 16;
    // TODO: For linked-list resolution, this really
    // should be reconsidered, and instead store
    // an array of pointers to `struct entry`s instead
    m->data = calloc(m->capacity, sizeof(*m->data));
    if (!m->data)
    {
        free(m);
        return NULL;
    }

    return m;
}

bool
map_insert(
    map * m,
    const char *key,
    double value)
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
                map_insert(m, target->key, target->value);
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
map_exists(
    map * m,
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

double
map_lookup(
    map * m,
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

    return NAN;
}

void
map_destroy(
    map * m)
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


void
hashtable_print(
    map * m)
{
    printf("SZ: %zu\tCAP: %zu\n", m->size, m->capacity);

    for (size_t n = 0; n < m->capacity; ++n)
    {
        printf("(%s, %lf)  ", m->data[n].key, m->data[n].value);
    }
    puts("");

}
