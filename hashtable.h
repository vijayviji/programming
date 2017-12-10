#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#include "common.h"
#include "list.h"

typedef struct HashItem {
   ListNode link;

   void *key;
   void *data;
} HashItem;

/*
 * bloom filter to avoid searches of non-existing data
 * 64 bytes is the size of bloom filter
 */
#define BLOOM_FILTER_BUCKETS_COUNT 1
#define BLOOM_FILTER_VECTOR_SIZE 64     // uint64_t

typedef struct HashBucket {
   List list;
   uint32_t num_keys;
   uint64_t bloom_bv[BLOOM_FILTER_BUCKETS_COUNT];  // bloom filter bit vectors
} HashBucket;

typedef struct HashStats {
   uint64_t search_hits;
   uint64_t search_misses;
   uint64_t search_list_walks; // due to collision, multiple elements will be in a chain in a bucket.
                               // these many no. of walks we did to find/not-find element
   uint64_t bloom_filter_saves;// how many times, bloom filter said the key is not found
   uint64_t inserts;
   uint64_t searches;
   uint64_t *collisions;
} HashStats;

// prints key and value
typedef void (*HTPrintKV) (void *key, void *data);

// compares two data. returns 0, if keys are equal
typedef int (*HTCompareData) (void *data1, void *data2);

// given a key, it gives an uint32_t value
typedef uint32_t (*HTHashFn) (void *key);

// Returns allocated key and copies the passed-in key. On OOM, returns NULL
typedef void * (*HTKeyAllocFn) (void *key);

// returns 0, if keys are equal
typedef int (*HTCompareKey) (void *key1, void *key2);

typedef struct HashTable {
   int num_buckets;
   HashBucket *buckets;

   HTHashFn hash_fn;                // converts key to bucket_no
   HTKeyAllocFn key_alloc_fn;       // Allocates a key
   HTCompareKey compare_key_fn;     // compares two keys
   HTCompareData compare_data_fn;   // compares two data
   HTPrintKV print_kv_fn;           // prints key and value
   HashStats stats;
} HashTable;

typedef HashTable * HTHandle;

/*
 * Hash functions Below
 *
 * hash functions should not mod the key since that will be done by hashtable.
 * the reason is we need to use it for bloom filter before mod'ing.
 */
static uint32_t ht_hash_from_web(uint32_t key)
{
   key = (key+0x7ed55d16) + (key<<12);
   key = (key^0xc761c23c) ^ (key>>19);
   key = (key+0x165667b1) + (key<<5);
   key = (key+0xd3a2646c) ^ (key<<9);
   key = (key+0xfd7046c5) + (key<<3);
   key = (key^0xb55a4f09) ^ (key>>16);
   return key;
}

static uint32_t ht_hash_simple(int key) {
   return key;
}

// taken from CLRS
static uint32_t ht_hash_clrs(uint32_t key)
{
   //(2654435769 / 2^32) is close to 0.6180339887 (suggested by Knuth)
   uint64_t tmp = key * (uint64_t) 2654435769;

   // tmp = (r0 * 2^32) + r1. To find r0, r1:
   uint32_t r0 = tmp % 4294967296;

   return r0;
}

static uint32_t sdbm(char *str)
{
    uint32_t hash = 0;
    int c;

    while (c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}
/*
 * Hash functions Above
 */

void ht_print_stats(HTHandle h);
void * ht_search_with_data_compare(HTHandle h_table, void *key, void *data);
void *ht_search(HTHandle h_table, void *key);
ReturnStatus ht_insert(HTHandle h_table, void *key, void *data);
void * ht_remove(HTHandle h_table, void *key, bool free_item);
void ht_print(HTHandle h_table);
HTHandle ht_alloc(int num_buckets, HTHashFn hash_fn, HTKeyAllocFn key_alloc_fn,
                  HTCompareKey compare_key_fn, HTCompareData compare_data_fn,
                  HTPrintKV print_kv_fn);
void ht_destroy(HTHandle h_table, bool destroy_data);

#endif // _HASHTABLE_H
