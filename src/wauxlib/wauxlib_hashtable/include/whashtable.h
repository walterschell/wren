#ifndef WHASHTABLE_H_
#define WHASHTABLE_H_
#include <stdbool.h>

typedef struct whashtable_t whashtable_t;
typedef void (*whashtable_free_value_f)(void *value);
typedef enum
{
    WHASHTABLE_SUCCESS = 0,
    WHASHTABLE_GEN_ERROR,
    WHASHTABLE_KEY_NOT_FOUND
} whashtable_status_t;

whashtable_t * whashtable_new(whashtable_free_value_f free_value);
void whashtable_delete(whashtable_t **table);

bool whashtable_contains(const whashtable_t *table, const char *key);
whashtable_status_t whashtable_set(const whashtable_t *table, const char *key, void *value);
whashtable_status_t whashtable_get(const whashtable_t *table, const char *key, void **value);
whashtable_status_t whashtable_del(const whashtable_t *table, const char *key);


#endif