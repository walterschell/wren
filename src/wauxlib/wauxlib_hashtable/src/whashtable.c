#include <whashtable.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include "whashtable_prime_index.h"



typedef struct whashtable_entry_t
{
    char *key;
    void *value;
    uint32_t hash;
    int distance_from_desired;
} whashtable_entry_t;

struct whashtable_t
{
    whashtable_free_value_f free_value;
    prime_index_t prime_index;
    size_t num_keys;
    whashtable_entry_t **buckets;
};

whashtable_t * whashtable_new(whashtable_free_value_f free_value)
{
    whashtable_t *result = malloc(sizeof(whashtable_t));
    if (NULL == result)
    {
        return result;
    }
    result->free_value = free_value;
    result->prime_index = WHASHTABLE_EMPTY;
    result->num_keys = 0;
    result->buckets = NULL;
    return result;
}
void whashtable_delete(whashtable_t **table)
{
    if (NULL == *table)
    {
        return;
    }



    free(*table);
    *table = NULL;

}
static whashtable_status_t lookup(whashtable_entry_t **entry, const whashtable_t *table, const char *key)
{
    if (table->num_keys == 0)
    {
        return WHASHTABLE_KEY_NOT_FOUND;
    }
    size_t bucket = lookup_bucket(hash(key), table->prime_index);
    unsigned int probes_left = max_probe_count[table->prime_index] + 1;
    do
    {
        if (NULL == table->buckets[bucket])
        {
            return WHASHTABLE_KEY_NOT_FOUND;
        }
        if (0 == strcmp(key, table->buckets[bucket]->key))
        {
            if (NULL != entry)
            {
                *entry = table->buckets[bucket];
            }
            return WHASHTABLE_SUCCESS;
        }
        probes_left--;
        bucket++;
    } while (probes_left != 0);
    return WHASHTABLE_KEY_NOT_FOUND;

}
bool whashtable_contains(const whashtable_t *table, const char *key)
{
    return (WHASHTABLE_SUCCESS == lookup(NULL, table, key));
}
whashtable_status_t whashtable_set(const whashtable_t *table, const char *key, void *value);
whashtable_status_t whashtable_get(const whashtable_t *table, const char *key, void **value)
{
    whashtable_entry_t *entry;
    if (WHASHTABLE_SUCCESS != lookup(&entry, table, key))
    {
        return WHASHTABLE_KEY_NOT_FOUND;
    }
    *value = entry->value;
    return WHASHTABLE_SUCCESS;
}
whashtable_status_t whashtable_del(const whashtable_t *table, const char *key);