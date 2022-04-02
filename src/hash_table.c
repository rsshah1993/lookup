#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hash_table.h"

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
static void ht_del(ht_hash_table *ht)
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
int ht_hash(char *s, const int a, const int m)
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

int main(void)
{
    ht_hash_table *ht = ht_new();
    ht_del(ht);
}
