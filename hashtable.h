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
   void *data;
   uint32_t key;
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

typedef void (*HTPrintItem) (void *);
typedef bool (*HTCompareData) (void *, void *);

typedef struct HashStats {
   uint64_t search_hits;
   uint64_t search_misses;
   uint64_t search_miss_walks; // hashCode didn't correspond directly to the key.
                               // We did this much walking to find key.
   uint64_t bloom_filter_saves;// how many times, bloom filter said the key is not found
   uint64_t inserts;
   uint64_t searches;
   uint64_t *collisions;
} HashStats;

typedef struct HashTable {
   int num_buckets;
   HashBucket *buckets;

   HTCompareData compare_data;
   HTPrintItem print_item;
   HashStats stats;
} HashTable;

typedef HashTable * HTHandle;


void ht_print_stats(HTHandle h);
void * ht_search_with_data_compare(HTHandle h_table, uint32_t key, void *data);
void *ht_search(HTHandle h_table, uint32_t key);
ReturnStatus ht_insert(HTHandle h_table, uint32_t key, void *data);
void * ht_remove(HTHandle h_table, uint32_t key, bool free_item);
void ht_print(HTHandle h_table);
HTHandle ht_alloc(int num_buckets, HTPrintItem print_item,
                  HTCompareData compare_data);
void ht_destroy(HTHandle h_table, bool destroy_data);

#endif // _HASHTABLE_H
