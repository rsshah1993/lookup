#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "hash_table.h"
#include "prime.h"

const int HT_PRIME1 = 153;
const int HT_PRIME2 = 163;
const int HT_INITIAL_BASE_SIZE = 53;
static ht_item HT_DELETED_ITEM = {NULL, NULL};

// initialize hash table item
static ht_item *ht_new_item(const char *k, const char *v)
{
    ht_item *i = malloc(sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

static ht_hash_table *ht_new_sized(const int base_size)
{
    ht_hash_table *ht = malloc(sizeof(ht_hash_table));
    ht->base_size = base_size;

    ht->size = next_prime(ht->base_size);
    ht->count = 0;

    ht->items = calloc((size_t)ht->size, sizeof(ht_item *));

    return ht;
}
// initialize new hash table
ht_hash_table *ht_new()
{
    ht_hash_table *ht = ht_new_sized(HT_INITIAL_BASE_SIZE);

    return ht;
}

// free hash table item memory
static void ht_del_item(ht_item *i)
{
    free(i->key);
    free(i->value);
    free(i);
}

// delete hash table and free all associated memory
void ht_delete_hash_table(ht_hash_table *ht)
{
    for (int i = 0; i < ht->size; i++)
    {
        ht_item *item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM)
        {
            ht_del_item(item);
        }
    }

    free(ht->items);
    free(ht);
}

static void ht_resize(ht_hash_table *ht, const int base_size)
{
    if (base_size < HT_INITIAL_BASE_SIZE)
    {
        return;
    }
    ht_hash_table *ht_new = ht_new_sized(base_size);
    for (int i = 0; i < ht->size; i++)
    {
        ht_item *item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM)
        {
            ht_insert(ht_new, item->key, item->value);
        }
    }
    ht->base_size = ht_new->base_size;
    ht->count = ht_new->count;

    const int tmp_size = ht->size;
    ht->size = ht_new->size;
    ht_new->size = tmp_size;

    ht_item **tmp_items = ht->items;
    ht->items = ht_new->items;
    ht_new->items = tmp_items;

    ht_delete_hash_table(ht_new);
}

static void ht_resize_up(ht_hash_table *ht)
{
    const int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}

static void ht_resize_down(ht_hash_table *ht)
{
    const int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}

int get_load(ht_hash_table *ht)
{
    const int load = (ht->count / ht->size) * 100;
    return load;
}

/*
Hashing function which hashes a string as follows:

    hash = ((a ** (len(s) - i + 1)) * s[i]) % m

For each character in s, where s is the string to be hashed, a is a
prime number larger than the total expected characters (in this
case we are using ASCII characters so a prime larger than 128),
and m is the number of hash buckets we would like.
*/
int ht_hash(const char *s, const int a, const int m)
{
    long hash = 0;
    const int len_s = strlen(s);
    for (int i = 0; i < len_s; i++)
    {
        hash += (long)pow(a, len_s - (i + 1)) * s[i];
        hash = hash % m;
    }
    return (int)hash;
}

/*
Using "double" method to resolve hashing collisions. Will hash the string twice
with different prime numbers in order to resolve conflicts.
*/
static int ht_get_hash(const char *s, const int num_buckets, const int attempt)
{
    const int hash_a = ht_hash(s, HT_PRIME1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME2, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

// insert item into hash table. Create's a hash item from the key/value
// pair and will hash the key. Will iteratively generate a new hash until
// there is an open bucket to insert the pair into.
// TODO: does not account for OOR on the current ht (which is initialized
// to size of 53)
void ht_insert(ht_hash_table *ht, const char *key, const char *value)
{
    int load = get_load(ht);
    if (load > 70)
    {
        ht_resize_up(ht);
    }

    ht_item *item = ht_new_item(key, value);
    int index = ht_get_hash(item->key, ht->size, 0);
    int i = 0;
    while (ht->items[index] != NULL)
    {
        index = ht_get_hash(item->key, ht->size, i);
    }
    ht->items[index] = item;
    ht->count++;
}

char *ht_search(ht_hash_table *ht, const char *key)
{
    int load = get_load(ht);
    if (load < 10)
    {
        ht_resize_down(ht);
    }

    int index = ht_get_hash(key, ht->size, 0);
    ht_item *item = ht->items[index];
    int i = 1;
    while (item != NULL)
    {
        if (strcmp(item->key, key) == 0)
        {
            return item->value;
        }
        else
        {
            index = ht_get_hash(key, ht->size, i);
            item = ht->items[index];
            i++;
        }
    }
    return NULL;
}

void ht_delete(ht_hash_table *ht, const char *key)
{
    int index = ht_get_hash(key, ht->size, 0);
    ht_item *item = ht->items[index];
    int i = 1;

    while (item != NULL)
    {
        if (item != &HT_DELETED_ITEM)
        {
            if (strcmp(item->key, key) == 0)
            {
                ht_del_item(item);
                // simply deleting from the ht will cause issues with our
                // collision chains. Replacing with a default deleted item
                // will allow us to maintain any collision chains that we
                // have previously developed.
                ht->items[index] = &HT_DELETED_ITEM;
            }
            index = ht_get_hash(key, ht->size, i);
            item = ht->items[index];
            i++;
        }
    }
    ht->count--;
}

void print_hash_map(ht_hash_table *ht)
{
    for (int i = 0; i < ht->size; i++)
    {
        ht_item *item = ht->items[i];
        if (item != NULL)
        {
            printf("Item %d has key: %s and value: %s\n", i, item->key, item->value);
        }
    }
}
