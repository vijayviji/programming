
typedef enum ReturnStatus {
   OK = 0,
   FAIL = 1,
   NO_MEM = 2
} ReturnStatus;

typedef struct ListNodeCustom {
   void *data;
   struct ListNodeCustom *prev;
   struct ListNodeCustom *next;
} ListNodeCustom;

#define LIST_FOR_ALL(list, node_ptr) \
   for (node_ptr = LIST_FIRST(list); \
        !LIST_IS_AT_END(list, node_ptr); \
        node_ptr = LIST_NEXT(node_ptr))

 // to be used when removing items in the list
#define LIST_FOR_ALL_SAFE(list, node_ptr, next_ptr) \
   for (node_ptr = LIST_FIRST(list), next_ptr = LIST_NEXT(LIST_FIRST(list)); \
        !LIST_IS_AT_END(list, node_ptr); \
        node_ptr = next_ptr, next_ptr = LIST_NEXT(next_ptr))

#define LIST_FIRST(list) (list)->head.next
#define LIST_LAST(list) (list)->head.prev
#define LIST_NEXT(node_ptr) (node_ptr)->next

#define LIST_IS_AT_END(list, node_ptr) ((node_ptr) == &((list)->head))
#define LIST_IS_EMPTY(list) ((list)->head.next == &(list->head))

typedef void (*ListPrintData)(void *);

typedef struct List {
   ListNodeCustom head;
   ListPrintData print_data_fn;
} List;

typedef List * ListHandle;


void list_insert_at_front(List *list, ListNodeCustom *node);
void list_insert_at_rear(List *list, ListNodeCustom *node);
void list_init_element(ListNodeCustom *node, void *data);
ListNodeCustom * list_alloc_element(void *data);
ReturnStatus list_insert_data(List *list, void *data, bool at_front);
void list_item_unlink(ListNodeCustom *node);
void list_item_free(ListNodeCustom *node, bool destroy_data);
void list_init(List *list, ListPrintData print_data_fn);
List * list_alloc(ListPrintData print_data_fn);
void list_destroy(List *list, bool destroy_data);
void list_print(List *list);


void list_insert_at_front(List *list, ListNodeCustom *node)
{
   ListNodeCustom *head = &list->head;

   node->next = head->next;
   head->next->prev = node;

   node->prev = head;
   head->next = node;
}

void list_insert_at_rear(List *list, ListNodeCustom *node)
{
   ListNodeCustom *head = &list->head;


   node->prev = head->prev;
   head->prev->next = node;

   node->next = head;
   head->prev = node;
}

void list_init_element(ListNodeCustom *node, void *data)
{
   node->next = NULL;
   node->prev = NULL;
   node->data = data;
}

ListNodeCustom * list_alloc_element(void *data)
{
   ListNodeCustom *tmp = malloc(sizeof(ListNodeCustom));

   if (!tmp) {
      return NULL;
   }

   list_init_element(tmp, data);
   return tmp;
}

ReturnStatus list_insert_data(List *list, void *data, bool at_front)
{
   ListNodeCustom *tmp = list_alloc_element(data);

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

void list_item_unlink(ListNodeCustom *node)
{
   node->prev->next = node->next;
   node->next->prev = node->prev;
}

void list_item_free(ListNodeCustom *node, bool destroy_data)
{
   list_item_unlink(node);
   if (destroy_data) {
      free(node);
   }
}

void list_init(List *list, ListPrintData print_data_fn)
{
   list->head.next = &list->head;
   list->head.prev = &list->head;
   list->print_data_fn = print_data_fn;
}

List * list_alloc(ListPrintData print_data_fn)
{
   List *list = malloc(sizeof(List));

   if (!list) {
      return NULL;
   }

   list_init(list, print_data_fn);
   return list;
}

void list_destroy(List *list, bool destroy_data)
{
   ListNodeCustom *itr, *next;

   if (!list) {
      return;
   }

   LIST_FOR_ALL_SAFE(list, itr, next) {
      list_item_free(itr, destroy_data);
   }

   free(list);
}

void list_print(List *list)
{
   ListNodeCustom *itr;

   if (!list) {
      return;
   }

   LIST_FOR_ALL(list, itr) {
      list->print_data_fn(itr->data);
      printf(" -> ");
   }

   printf("\n");
}



typedef struct HashItem {
   ListNodeCustom link;
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

#define HT_BLOOM_BUCKET(key)  \
   ((key) % BLOOM_FILTER_BUCKETS_COUNT)

#define HT_BLOOM_TESTSET(bloom_bv, key)    \
   ((bloom_bv)[HT_BLOOM_BUCKET((key))] & (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE)))

#define HT_BLOOM_SET(bloom_bv, key)    \
   (bloom_bv)[HT_BLOOM_BUCKET((key))] |= (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE))

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


void ht_print_stats(HTHandle h)
{
   printf("HashTable: search_hits - %u, search_misses - %u,"
          " search_miss_walks - %u, bloom_filter_saves - %u,"
          " searches - %u, inserts - %u\n",
          h->stats.search_hits, h->stats.search_misses,
          h->stats.search_miss_walks, h->stats.bloom_filter_saves,
          h->stats.searches, h->stats.inserts);

   printf("Max collided elemetns in a row: %d\n",
          find_max(h->stats.collisions, h->num_buckets));
}

// uint32_t hashCode(HTHandle h_table, uint32_t key)
// {
//    key = (key+0x7ed55d16) + (key<<12);
//    key = (key^0xc761c23c) ^ (key>>19);
//    key = (key+0x165667b1) + (key<<5);
//    key = (key+0xd3a2646c) ^ (key<<9);
//    key = (key+0xfd7046c5) + (key<<3);
//    key = (key^0xb55a4f09) ^ (key>>16);
//    return key % h_table->num_buckets;
// }

// int hashCode(HTHandle h_table, int key) {
//    return key % h_table->num_buckets;
// }

// taken from CLRS
uint32_t hashCode(HTHandle h_table, uint32_t key)
{
   //(2654435769 / 2^32) is close to 0.6180339887 (suggested by Knuth)
   uint64_t tmp = key * (uint64_t) 2654435769;

   // tmp = (r0 * 2^32) + r1. To find r0, r1:
   uint32_t r0 = tmp % 4294967296;

   return r0 % h_table->num_buckets;
}

 /**
  * if Keys match, it compares data also.
  */
void * ht_search_with_data_compare(HTHandle h_table, uint32_t key, void *data)
{
   int hashIndex = hashCode(h_table, key);
   ListNodeCustom *itr;
   HashItem *item;

   h_table->stats.searches++;

   if (!HT_BLOOM_TESTSET(h_table->buckets[hashIndex].bloom_bv, key)) {
      h_table->stats.bloom_filter_saves++;
      h_table->stats.search_misses++;
      return NULL;   // bloom filter says the key is not there.
   }

   LIST_FOR_ALL(&h_table->buckets[hashIndex].list, itr) {
      item = (HashItem *) itr;
      if (item->key == key) {
         h_table->stats.search_hits++;

         if (!data) {
            return item->data;
         } else if (h_table->compare_data(data, item->data)) { // compare data
            return item->data;
         } else {
            return NULL;
         }
         break;
      }
      h_table->stats.search_miss_walks++;
   }

   // key not found
   h_table->stats.search_misses++;
   return NULL;
}


void *ht_search(HTHandle h_table, uint32_t key)
{
   return ht_search_with_data_compare(h_table, key, NULL);
}

ReturnStatus ht_insert(HTHandle h_table, uint32_t key, void *data)
{
   //get the hash
   int hashIndex = hashCode(h_table, key);
   HashItem *item = NULL;

   h_table->stats.inserts++;

   if (h_table->buckets[hashIndex].num_keys > 0) {
      h_table->stats.collisions[hashIndex]++;
   }

   item = malloc(sizeof(HashItem));
   if (!item) {
      return NO_MEM;
   }

   list_init_element(&item->link, NULL);
   item->key = key;
   item->data = data;
   list_insert_at_front(&h_table->buckets[hashIndex].list, &item->link);
   h_table->buckets[hashIndex].num_keys++;

   HT_BLOOM_SET(h_table->buckets[hashIndex].bloom_bv, key);

   return OK;
}

void * ht_remove(HTHandle h_table, uint32_t key, bool free_item)
{
   int hashIndex = hashCode(h_table, key);
   HashItem *item = NULL;
   ListNodeCustom *itr = NULL;
   void *data = NULL;

   LIST_FOR_ALL(&h_table->buckets[hashIndex].list, itr) {
      item = (HashItem *) itr;
      if (item->key == key) {
         list_item_unlink(&item->link);
         data = item->data;
         if (free_item) {
            free(data);
            data = NULL;
         }
         free(item);
         item = NULL;
         h_table->buckets[hashIndex].num_keys--;
         break;
      }
   }

   return data;
}

void ht_print(HTHandle h_table)
{
   int i = 0;
   ListNodeCustom *itr;
   HashItem *item;

   for (i = 0; i < h_table->num_buckets; i++) {
      LIST_FOR_ALL(&h_table->buckets[i].list, itr) {
         item = (HashItem *) itr;
         printf(" (%d,", item->key);
         h_table->print_item(item->data);
         printf(") ->");
      }
      printf(" ~~ \n");
   }

   printf("\n");
}

HTHandle ht_alloc(int num_buckets, HTPrintItem print_item,
                  HTCompareData compare_data)
{
   HTHandle h_table;
   int i;

   h_table = (HTHandle) malloc(sizeof(HashTable));
   if (!h_table) {
      return NULL;
   }
   memset(h_table, 0, sizeof(HashTable));

   h_table->num_buckets = num_buckets;

   h_table->buckets = (HashBucket *) malloc(sizeof(HashBucket) * num_buckets);
   if (!h_table->buckets) {
      goto free_table;
   }
   memset(h_table->buckets, 0, sizeof(HashBucket) * num_buckets);

   for (i = 0; i < num_buckets; i++) {
      h_table->buckets[i].num_keys = 0;
      list_init(&h_table->buckets[i].list, NULL);
   }

   h_table->stats.collisions = malloc(sizeof(uint64_t) * num_buckets);
   memset(h_table->stats.collisions, 0, sizeof(uint64_t) * num_buckets);

   h_table->print_item = print_item;
   h_table->compare_data = compare_data;

   return h_table;

free_table:
   free(h_table);
   return NULL;
}

void ht_destroy(HTHandle h_table, bool destroy_data)
{
   int i;
   void *data;
   ListNodeCustom *itr;
   HashItem *item;

   for (i = 0; i < h_table->num_buckets; i++) {
      LIST_FOR_ALL(&h_table->buckets[i].list, itr) {
         item = (HashItem *) itr;
         data = ht_remove(h_table, item->key, destroy_data);
      }
   }

   free(h_table);
}



typedef struct LRUCache {
   uint64_t capacity;
   uint64_t capacity_used;

   HTHandle hash_table;    // index of the elements in list
   ListHandle list;
} LRUCache;

typedef struct LRUCacheItem {
   ListNodeCustom link; // this can hold data
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

   lRUCachePrint(cache, true);

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
   ListNodeCustom *list_itr;
   LRUCacheItem *item;

   if (!cache) {
      return;
   }

   // printf("Printing List:\n");
   //
   // LIST_FOR_ALL(cache->list, list_itr) {
   //    item = LINK_TO_CACHE_ITEM(list_itr);
   //    printf("%d -> %d\n", item->key, *(int *)item->link.data);
   // }
   //
   // printf("\nPrinting Hash Table\n");
   // ht_print(cache->hash_table);

   if (print_stats) {
      ht_print_stats(cache->hash_table);
   }
}
