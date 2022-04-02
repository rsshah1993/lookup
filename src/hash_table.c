#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "hash_table.h"

const int HT_PRIME1 = 153;
const int HT_PRIME2 = 163;

// initialize hash table item
static ht_item *ht_new_item(const char *k, const char *v)
{
    ht_item *i = malloc(sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

// initialize new hash table
ht_hash_table *ht_new()
{
    ht_hash_table *ht = malloc(sizeof(ht_hash_table));

    ht->size = 53;
    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(ht_item *));

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
static void ht_delete_hash_table(ht_hash_table *ht)
{
    for (int i = 0; i < ht->size; i++)
    {
        ht_item *item = ht->items[i];
        if (item != NULL)
        {
            ht_del_item(item);
        }
    }

    free(ht->items);
    free(ht);
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

static ht_item HT_DELETED_ITEM = {NULL, NULL};
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
int main(void)
{
    ht_hash_table *ht = ht_new();
    ht_insert(ht, "cat", "cats");
    ht_insert(ht, "dog", "dogs");
    ht_insert(ht, "spider", "spiders");
    print_hash_map(ht);
    ht_delete_hash_table(ht);
}
