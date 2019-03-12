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
    int distance_from_desired;
} whashtable_entry_t;

struct whashtable_t
{
    whashtable_free_value_f free_value;
    whashtable_prime_index_t prime_index;
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

bool whashtable_contains(const whashtable_t *table, const char *key);
whashtable_status_t whashtable_set(const whashtable_t *table, const char *key, void *value);
whashtable_status_t whashtable_get(const whashtable_t *table, const char *key, void **value);
whashtable_status_t whashtable_del(const whashtable_t *table, const char *key);