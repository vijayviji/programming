#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdint.h>
#include <assert.h>

#include "../list.h"
#include "../hashtable.h"

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
      tmp = LINK_TO_CACHE_ITEM(cache->list->head.prev);
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
   ht_print(cache->hash_table);

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
