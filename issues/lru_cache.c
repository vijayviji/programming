#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdint.h>
#include <assert.h>

typedef enum ReturnStatus {
   OK = 0,
   FAIL = 1,
   NO_MEM = 2
} ReturnStatus;

typedef struct ListNode {
   void *data;
   struct ListNode *prev;
   struct ListNode *next;
} ListNode;

#define LIST_FOR_ALL(list, node_ptr) \
   for (node_ptr = LIST_FIRST(list); \
        !LIST_IS_AT_END(list, node_ptr); \
        node_ptr = LIST_NEXT(node_ptr))

// to be used when removing items in the list
#define LIST_FOR_ALL_SAFE(list, node_ptr, next_ptr) \
   for (node_ptr = LIST_FIRST(list), next_ptr = LIST_NEXT(LIST_FIRST(list)); \
        !LIST_IS_AT_END(list, node_ptr); \
        node_ptr = next_ptr, next_ptr = LIST_NEXT(next_ptr))

#define LIST_FIRST(list) list->head->next
#define LIST_LAST(list) list->head->prev
#define LIST_NEXT(node_ptr) node_ptr->next

#define LIST_IS_AT_END(list, node_ptr) (node_ptr == list->head)
#define LIST_IS_EMPTY(list) (list->head->next == list->head)

typedef void (*ListPrintData)(void *);

typedef struct List {
   ListNode *head;
   ListPrintData print_data_fn;
} List;

typedef List * ListHandle;

void list_insert_at_front(ListHandle list, ListNode *node)
{
   ListNode *head = list->head;


   node->next = head->next;
   head->next->prev = node;

   node->prev = head;
   head->next = node;
}

void list_insert_at_rear(ListHandle list, ListNode *node)
{
   ListNode *head = list->head;


   node->prev = head->prev;
   head->prev->next = node;

   node->next = head;
   head->prev = node;
}

void list_init_element(ListNode *node, void *data)
{
   node->next = node->prev = NULL;
   node->data = data;
}

ListNode * list_alloc_element(void *data)
{
   ListNode *tmp = malloc(sizeof(ListNode));

   if (!tmp) {
      return NULL;
   }

   list_init_element(tmp, data);

   return tmp;
}

ReturnStatus list_insert_data(ListHandle list, void *data, bool at_front)
{
   ListNode *tmp = list_alloc_element(data);

   if (!tmp) {
      return NO_MEM;
   }

   if (at_front) {
      list_insert_at_front(list, tmp);
   } else {
      list_insert_at_rear(list, tmp);
   }

   return OK;
}

void list_item_unlink(ListNode *node)
{
   node->prev->next = node->next;
   node->next->prev = node->prev;
}

void list_item_free(ListNode *node, bool destroy_data)
{
   list_item_unlink(node);
   if (destroy_data) {
      free(node);
   }
}

ListHandle list_alloc(ListPrintData print_data_fn)
{
   ListHandle list = malloc(sizeof(List));
   ListNode *head;

   if (!list) {
      return NULL;
   }

   head = list_alloc_element((void *) NULL);
   if (!head) {
      free(list);
      return NULL;
   }

   head->next = head;
   head->prev = head;

   list->head = head;
   list->print_data_fn = print_data_fn;

   return list;
}

void list_destroy(List *list, bool destroy_data)
{
   ListNode *itr, *next;

   if (!list) {
      return;
   }

   LIST_FOR_ALL_SAFE(list, itr, next) {
      list_item_free(itr, destroy_data);
   }

   free(list);
}

void list_print(ListHandle list)
{
   ListNode *itr;

   if (!list) {
      return;
   }

   LIST_FOR_ALL(list, itr) {
      list->print_data_fn(itr->data);
      printf(" -> ");
   }

   printf("\n");
}


// below for hashtable

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

typedef struct HashTable {
   int num_keys;
   HashItem *table;

   HTCompareData compare_data;
   HTPrintItem print_item;
   HashStats stats;

   // bloom filter to avoid searches of non-existing data
#define BLOOM_FILTER_BUCKETS_COUNT 1000 // 62.5 KiB is the size of bloom filter
#define BLOOM_FILTER_VECTOR_SIZE 64     // uint64_t
   uint64_t *bloom_bv;  // bloom filter bit vector
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

uint32_t hashCode(HTHandle h_table, uint32_t key)
{
   // taken from CLRS
   uint64_t tmp = key * (uint64_t) 2654435769; //2654435769 / 2^32 close to 0.6180339887 (suggested by Knuth)

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


/***
 * Below is lru_cache stuff
 */

typedef struct LRUCache {
   uint64_t capacity;
   uint64_t capacity_used;

   HTHandle hash_table;    // index of the elements in list
   ListHandle list;
} LRUCache;

typedef struct LRUCacheItem {
   ListNode link; // this can hold data
   uint32_t key;
} LRUCacheItem;

#define LINK_TO_CACHE_ITEM(link) ((LRUCacheItem *) (link))

typedef LRUCache * LRUCacheHandle;


LRUCacheItem * lRUCacheAllocItem(uint32_t key, void *data)
{
   LRUCacheItem *tmp = malloc(sizeof(LRUCacheItem));

   if (!tmp) {
      return tmp;
   }

   tmp->key = key;
   list_init_element(&tmp->link, data);

   return tmp;
}


void hash_print_int_item(void *data)
{
   LRUCacheItem *item = (LRUCacheItem *) data;

   printf("%d", *(int *)item->link.data);
}

LRUCacheHandle lRUCacheCreate(uint64_t capacity)
{
   LRUCacheHandle cache = NULL;

   if (capacity == 0) {
      return NULL;
   }

   cache = malloc(sizeof(LRUCache));

   if (!cache) {
      return NULL;
   }

   cache->capacity = capacity;
   cache->capacity_used = 0;

   cache->hash_table = ht_alloc(capacity, hash_print_int_item, NULL);
   if (!cache->hash_table) {
      goto free_cache;
   }

   cache->list = list_alloc(NULL);
   if (!cache->list) {
      goto free_hash_table;
   }

   return cache;

free_hash_table:
   free(cache->hash_table);
free_cache:
   free(cache);

   return NULL;
}

void lRUCacheFree(LRUCacheHandle cache)
{

   if (!cache) {
      return;
   }

   list_destroy(cache->list, false);
   //printf("Destroyed list\n");

   ht_destroy(cache->hash_table, true);
   //printf("Destroyed hash_table along with data\n");
   free(cache);
}

LRUCacheItem * lRUCacheGetItem(LRUCacheHandle cache, uint32_t key) {
   LRUCacheItem *item;

   if (!cache) {
      return NULL;
   }

   item = ht_search(cache->hash_table, key);
   if (!item) {
      return NULL;
   }

   list_item_unlink(&item->link);
   list_insert_at_front(cache->list, &item->link);

   return item;
}

int lRUCacheGet(LRUCacheHandle cache, uint32_t key) {
   LRUCacheItem *item;

   item = lRUCacheGetItem(cache, key);
   if (!item) {
      return -1;
   }

   return *((int *)item->link.data);
}

void lRUCachePut(LRUCacheHandle cache, uint32_t key, int value) {
   LRUCacheItem *item = NULL, *tmp = NULL;
   int *data = NULL;

   if (!cache || cache->capacity == 0) {
      return;
   }

   item = lRUCacheGetItem(cache, key);
   if (item) {
      // key is already there. Just set it in that case.
      data = item->link.data;
      *data = value;
      return;
   }

   data = malloc(sizeof(int));
   if (!data) {
      return;
   }

   *data = value;

   item = lRUCacheAllocItem(key, data);
   if (!item) {
      free(data);
      return;
   }

   if (cache->capacity_used == cache->capacity) {
      tmp = LINK_TO_CACHE_ITEM(cache->list->head->prev);
      // ("Removing %d\n", tmp->key);
      list_item_unlink(&tmp->link);
      ht_remove(cache->hash_table, tmp->key, true);
      tmp = NULL;
      cache->capacity_used--;
   }

   cache->capacity_used++;
   // ("capacity_used: %d\n", cache->capacity_used);

   list_insert_at_front(cache->list, &item->link);
   ht_insert(cache->hash_table, key, item);
}

void lRUCachePrint(LRUCacheHandle cache, bool print_stats)
{
   ListNode *list_itr;
   LRUCacheItem *item;

   if (!cache) {
      return;
   }

   printf("Printing List:\n");

   LIST_FOR_ALL(cache->list, list_itr) {
      item = LINK_TO_CACHE_ITEM(list_itr);
      printf("%d -> %d\n", item->key, *(int *)item->link.data);
   }

   printf("\nPrinting Hash Table\n");
   ht_display(cache->hash_table);

   if (print_stats) {
      ht_print_stats(cache->hash_table);
   }
}



// below for testing
int main()
{
   LRUCacheHandle cache = lRUCacheCreate(10);
   int i;

   if (!cache) {
      printf("cache alloc failed\n");
      return 1;
   }

   for (i = 1; i <= 100; i++) {
      printf("Getting %d - %d\n", 2, lRUCacheGet(cache, 2));
      printf("Putting %d, value: %d\n", i, i);
      lRUCachePut(cache, i, i);
   }

   printf("Printing the cache:\n");
   lRUCachePrint(cache, true);
   lRUCacheFree(cache);

   return 0;
}
