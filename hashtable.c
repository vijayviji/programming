#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

uint64_t find_max(uint64_t *arr, uint64_t count)
{
   uint64_t i;
   uint64_t max = 0;

   for (i = 0; i < count; i++) {
      if (arr[i] > max) {
         max = arr[i];
      }
   }

   return max;
}

#define HASH_INIT_MAGIC 0xf005ba11

typedef struct HashItem {
   void *data;
   uint32_t key;
   bool is_key_valid;
} HashItem;


typedef void (*HTPrintItem) (void *);
typedef bool (*HTCompareData) (void *, void *);

typedef struct HashStats {
   uint64_t search_hits;
   uint64_t search_misses;
   uint64_t search_direct_hit;        // hashCode corresponded directly to the key in
                                      // hash table
   uint64_t search_direct_misses;     // hashCode didn't correspond directly to the key.
   uint64_t search_direct_miss_walks; // hashCode didn't correspond directly to the key.
                                      // We did this much walking to find key.
   uint64_t bloom_filter_saves;       // how many times, bloom filter said the key is not found
   uint64_t inserts;
   uint64_t searches;
   uint64_t *collisions;
} HashStats;

/*
 * bloom filter to avoid searches of non-existing data
 * 64 KiB is the size of bloom filter
 */
#define BLOOM_FILTER_BUCKETS_COUNT 1024
#define BLOOM_FILTER_VECTOR_SIZE 64     // uint64_t

typedef struct HashTable {
   int num_keys;
   HashItem *table;
   uint64_t *bloom_bv;  // bloom filter bit vector

   HTCompareData compare_data;
   HTPrintItem print_item;
   HashStats stats;
} HashTable;

typedef HashTable * HTHandle;


void ht_print_stats(HTHandle h)
{
   printf("HashTable: search_hits - %u, search_misses - %u,"
          " search_direct_hit - %u, search_direct_misses - %u,"
          " search_direct_miss_walks - %u, bloom_filter_saves - %u,"
          " searches - %u, inserts - %u\n",
          h->stats.search_hits, h->stats.search_misses,
          h->stats.search_direct_hit, h->stats.search_direct_misses,
          h->stats.search_direct_miss_walks,  h->stats.bloom_filter_saves,
          h->stats.searches, h->stats.inserts);

   printf("Max collided elemetns in a row: %d\n",
          find_max(h->stats.collisions, h->num_keys));
}

#define HT_BLOOM_BUCKET(key)  \
   ((key) % BLOOM_FILTER_BUCKETS_COUNT)

#define HT_BLOOM_TESTSET(h_table, key)    \
   ((h_table)->bloom_bv[HT_BLOOM_BUCKET((key))] & (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE)))

#define HT_BLOOM_SET(h_table, key)    \
   (h_table)->bloom_bv[HT_BLOOM_BUCKET((key))] |= (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE))

// uint32_t hashCode(HTHandle h_table, uint32_t key)
// {
//    key = (key+0x7ed55d16) + (key<<12);
//    key = (key^0xc761c23c) ^ (key>>19);
//    key = (key+0x165667b1) + (key<<5);
//    key = (key+0xd3a2646c) ^ (key<<9);
//    key = (key+0xfd7046c5) + (key<<3);
//    key = (key^0xb55a4f09) ^ (key>>16);
//    return key % h_table->num_keys;
// }

// taken from CLRS
uint32_t hashCode(HTHandle h_table, uint32_t key)
{
   //(2654435769 / 2^32) is close to 0.6180339887 (suggested by Knuth)
   uint64_t tmp = key * (uint64_t) 2654435769;

   // tmp = (r0 * 2^32) + r1. To find r0, r1:
   uint32_t r0 = tmp % 4294967296;

   return r0 % h_table->num_keys;
}

// int hashCode(HTHandle h_table, int key) {
//    return key % h_table->num_keys;
// }

 /**
  * if Keys match, it compares data also.
  */
 void *ht_search_with_data_compare(HTHandle h_table, uint32_t key, void *data)
 {
    //get the hash
    int hashIndex = hashCode(h_table, key), originalHashIndex;

    originalHashIndex = hashIndex;

    h_table->stats.searches++;

   if (h_table->table[hashIndex].key == key) {
      h_table->stats.search_direct_hit++;
   } else {
      h_table->stats.search_direct_misses++;

      //printf("hash direct miss key: %d\n", key);

      if (!HT_BLOOM_TESTSET(h_table, key)) {
         h_table->stats.bloom_filter_saves++;
         h_table->stats.search_misses++;
         return NULL;   // bloom filter says the key is not there.
      }

      while(h_table->table[hashIndex].key != key) {
         h_table->stats.search_direct_miss_walks++;
         //go to next cell
         ++hashIndex;
         //wrap around the table
         hashIndex %= h_table->num_keys;

         // we searched entire table
         if (hashIndex == originalHashIndex) {
            h_table->stats.search_misses++;
            return NULL;
         }
      }
   }

   // key is found

   if (data == NULL) {
      h_table->stats.search_hits++;
      return h_table->table[hashIndex].data;
   } else if (h_table->compare_data(data,
              h_table->table[hashIndex].data)) {
      h_table->stats.search_hits++;
      return h_table->table[hashIndex].data;
   }

   h_table->stats.search_misses++;
   return NULL;
 }


void *ht_search(HTHandle h_table, uint32_t key) {
   return ht_search_with_data_compare(h_table, key, NULL);
}

void ht_insert(HTHandle h_table, uint32_t key, void *data)
{
   //get the hash
   int hashIndex = hashCode(h_table, key), originalHashIndex;
   originalHashIndex = hashIndex;

   h_table->stats.inserts++;

   if (h_table->table[hashIndex].is_key_valid) {
      h_table->stats.collisions[hashIndex]++;
   }

   //move in array until an empty or deleted cell
   while(h_table->table[hashIndex].is_key_valid) {
      //go to next cell
      hashIndex++;
      //wrap around the table
      hashIndex %= h_table->num_keys;

      // we searched entire table
      if (hashIndex == originalHashIndex) {
         // ("TABLE IS FULL\n");
         return;
      }
   }

   // space is found

   HT_BLOOM_SET(h_table, key);
   h_table->table[hashIndex].key = key;
   h_table->table[hashIndex].is_key_valid = true;
   h_table->table[hashIndex].data = data;
}

void * ht_remove(HTHandle h_table, uint32_t key, bool free_item)
{
   //get the hash
   int hashIndex = hashCode(h_table, key), originalHashIndex;
   void *data;

   originalHashIndex = hashIndex;

   //move in array until an empty
   while(h_table->table[hashIndex].key != key) {
      //go to next cell
      hashIndex++;
      //wrap around the table
      hashIndex %= h_table->num_keys;

      // we searched entire table
      if (hashIndex == originalHashIndex) {
         // ("KEY NOT FOUND\n");
         return;
      }
   }

   // key found

   data = h_table->table[hashIndex].data;
   if (free_item) {
     free(data);
     data = NULL;
   }

   h_table->table[hashIndex].is_key_valid = false;
   h_table->table[hashIndex].key = HASH_INIT_MAGIC;
   h_table->table[hashIndex].data = NULL;

   return data;
}

void ht_display(HTHandle h_table) {
   int i = 0;

   for(i = 0; i < h_table->num_keys; i++) {
     if(h_table->table[i].is_key_valid) {
        printf(" (%d,", h_table->table[i].key);
        h_table->print_item(h_table->table[i].data);
        printf(")\n");
     } else {
        printf(" ~~ \n");
     }
   }

   printf("\n");
}

HTHandle ht_alloc(int num_keys, HTPrintItem print_item, HTCompareData compare_data)
{
   HTHandle h_table;
   int i;

   h_table = (HTHandle) malloc(sizeof(HashTable));
   memset(h_table, 0, sizeof(HashTable));

   h_table->num_keys = num_keys;

   h_table->table = (HashItem *) malloc(sizeof(HashItem) * num_keys);

   h_table->stats.collisions = malloc(sizeof(uint64_t) * num_keys);
   memset(h_table->stats.collisions, 0, sizeof(uint64_t) * num_keys);

   h_table->print_item = print_item;
   h_table->compare_data = compare_data;

   for (i = 0; i < num_keys; i++) {
      h_table->table[i].key = HASH_INIT_MAGIC;
      h_table->table[i].is_key_valid = false;
      h_table->table[i].data = NULL;
   }

   h_table->bloom_bv = malloc(BLOOM_FILTER_BUCKETS_COUNT * sizeof(uint64_t));

   return h_table;
}

void ht_destroy(HTHandle h_table, bool destroy_data)
{
   int i;
   void *data;

   for (i = 0; i < h_table->num_keys; i++) {
      if (h_table->table[i].is_key_valid) {
         data = ht_remove(h_table, h_table->table[i].key, destroy_data);
         assert(data == NULL);
      }
   }

   free(h_table->bloom_bv);

   free(h_table);
}

/*
 * Below are for testing
 */

void print_int_item(void *data)
{
   int *item = (int *) data;

   printf("%d", *item);
}

bool compare_int_data(void *data1, void *data2)
{
   int val1 = *(int *) data1, val2 = *(int *) data2;

   return val1 == val2;
}


#define MAX 1000
int main()
{
   HTHandle h_table = ht_alloc(MAX, print_int_item, compare_int_data);
   int *data, i, keys[MAX];

   srand(time(NULL));   // should only be called once

   for (i = 0; i < MAX; i++) {
      data = malloc(sizeof(int));

      *data = i;
      keys[i] = rand(); // returns a pseudo-random integer between 0 and RAND_MAX
      ht_insert(h_table, keys[i], data);
   }

   for (i = 0; i < MAX; i++) {
      data = (int *) ht_search(h_table, keys[i]);
      if (!data) {
         printf("search: %d. result: NULL\n", keys[i]);
      } else {
         printf("search: %d. result: %d\n", keys[i], *data);
      }
   }

   printf("\nSearching for keys that are not there:\n");

   for (i = 0; i < MAX; i++) {
      data = (int *) ht_search(h_table, keys[i] + 1);
      if (!data) {
         printf("search: %d. result: NULL\n", keys[i] + 1);
      } else {
         printf("search: %d. result: %d\n", keys[i] + 1, *data);
      }
   }

   data = ht_remove(h_table, keys[i - 1], false);
   free(data);
   ht_display(h_table);
   ht_print_stats(h_table);
   ht_destroy(h_table, true);
}
