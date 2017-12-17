#ifndef _COMMON_H
#define _COMMON_H


typedef enum ReturnStatus {
   OK = 0,
   FAIL = 1,
   NO_MEM = 2
} ReturnStatus;

#define MAX(a, b) (((a) > (b))? (a): (b))
#define MIN(a, b) (((a) < (b))? (a): (b))

static uint64_t find_max(uint64_t *arr, uint64_t count)
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

static char * substring(char source[], int start, int len) {
   int i = start;

   char *target = malloc(len);
   if (!target) {
      return NULL;
   }

   while (i < len) {
      target[i] = source[i];
      i++;
   }
   target[i] = '\0';

   return target;
}

// in-place reverse.
static char * str_reverse(char *str)
{
   int i, len;
   char tmp;

    // calculating length of the string
   len = strlen(str);

   for(i = 0; i < len / 2; i++) {
      tmp = str[i];
      str[i] = str[len - i - 1];
      str[len - i - 1] = tmp;
   }

   return str;
}

// copies and then reverses.
static char * str_reverse_cpy(char *str)
{
   int i, len;
   char tmp;
   char *ret;

   len = strlen(str);
   ret = malloc(len + 1);
   assert(ret != NULL);
   strcpy(ret, str);

   return str_reverse(ret);
}

// i and j are inclusive
static bool is_palindrome_ij(char *s, int i, int j)
{
   for (; i < j; i++, j--) {
      if (s[i] != s[j]) {
         return false;
      }
   }

   return true;
}

static bool is_palindrome(char *s)
{
   int l = strlen(s), i;

   return is_palindrome_ij(s, 0, l - 1);
}

#endif // _COMMON_H
#ifndef _LIST_H
#define _LIST_H



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

#define LIST_FIRST(list) (list)->head.next
#define LIST_LAST(list) (list)->head.prev
#define LIST_NEXT(node_ptr) (node_ptr)->next

#define LIST_IS_AT_END(list, node_ptr) ((node_ptr) == &((list)->head))
#define LIST_IS_EMPTY(list) ((list)->head.next == &(list->head))

typedef void (*ListPrintData)(void *);

typedef struct List {
   ListNode head;
   ListPrintData print_data_fn;
} List;

typedef List * ListHandle;


void list_insert_at_front(List *list, ListNode *node);
void list_insert_at_rear(List *list, ListNode *node);
void list_init_element(ListNode *node, void *data);
void list_make_element_self_ref(ListNode *node);
ListNode * list_alloc_element(void *data);
ReturnStatus list_insert_data(List *list, void *data, bool at_front);
void list_item_unlink(ListNode *node);
void list_items_link(ListNode *node1, ListNode *node2);
void list_item_free(ListNode *node, bool destroy_data);
void list_init(List *list, ListPrintData print_data_fn);
List * list_alloc(ListPrintData print_data_fn);
void list_destroy(List *list, bool destroy_data);
void list_print(List *list);

#endif // _LIST_H

void list_insert_at_front(List *list, ListNode *node)
{
   ListNode *head = &list->head;

   node->next = head->next;
   head->next->prev = node;

   node->prev = head;
   head->next = node;
}

void list_insert_at_rear(List *list, ListNode *node)
{
   ListNode *head = &list->head;


   node->prev = head->prev;
   head->prev->next = node;

   node->next = head;
   head->prev = node;
}

void list_init_element(ListNode *node, void *data)
{
   node->next = NULL;
   node->prev = NULL;
   node->data = data;
}

void list_make_element_self_ref(ListNode *node)
{
   node->next = node;
   node->prev = node;
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

ReturnStatus list_insert_data(List *list, void *data, bool at_front)
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

void list_items_link(ListNode *node1, ListNode *node2)
{
   ListNode *node1Next = node1->next, *node2Prev = node2->prev;

   node1->next = node2;
   node1Next->prev = node2Prev;

   node2->prev = node1;
   node2Prev->next = node1Next;
}

void list_item_free(ListNode *node, bool destroy_data)
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
   ListNode *itr, *next;

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
#ifndef _HASHTABLE_H
#define _HASHTABLE_H



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



// key and data are ints

static uint32_t ht_hash_clrs_int_fn(void *key)
{
   return ht_hash_clrs(*(int *) key);
}

static void * key_alloc_fn_int_fn(void *key)
{
   int *int_key = (int *) key;
   int *new_key = malloc(sizeof(int));

   if (new_key) {
      memcpy(new_key, int_key, sizeof(int));
   }

   return new_key;
}

static int compare_key_int_fn(void *key1, void *key2)
{
   int *intkey1 = (int *) key1, *intkey2 = (int *) key2;

   return *intkey1 - *intkey2;
}

static int compare_data_int_fn(void *data1, void *data2)
{
   int *val1 = (int *) data1, *val2 = (int *) data2;

   return (*val1 - *val2);
}

static void print_kv_int_fn(void *key, void *data)
{
   int *int_key = (int *) key;
   int *int_data = (int *) data;

   printf("%d, %d", *int_key, *int_data);
}




// key and data are string
static uint32_t sdbm_str_fn(void *key)
{
   return sdbm(key);
}

static void * key_alloc_fn_str(void *key)
{
   char *strkey = (char *) key;
   uint32_t len = strlen(strkey);
   char *new_key = malloc(len + 1);

   if (new_key) {
      memcpy(new_key, strkey, len + 1);
   }

   return new_key;
}

static int compare_key_str_fn(void *key1, void *key2)
{
   char *strkey1 = (char *) key1, *strkey2 = (char *) key2;

   return strcmp(strkey1, strkey2);
}

static int compare_data_str_fn(void *data1, void *data2)
{
   char *val1 = (char *) data1, *val2 = (char *) data2;

   return strcmp(val1, val2);
}

static void print_kv_str_fn(void *key, void *data)
{
   char *str_key = (char *) key;
   char *str_data = (char *) data;

   printf("%s, %s", str_key, str_data);
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

/*
 * Hash Table can be implemented as:
 * ** Direct Addressing
 * ** Open Addressing
 * ** Chaining
 * ** Perfect Hashing
 *
 * Direct Addressing means, the key will be index into the hash table array.
 * For this case, the key has to fit within the size of the array.
 * Worst case: O(1). Avg case: O(1)
 *
 * For other hashing techniques, we use a hash function on top of key.
 * new_key = hash(key). Because we do this, collision can happen and we can solve
 * collision by three methods: Open Addressing, Chaining, Perfect Hashing
 *
 * Chaining: When collision happens (two keys collide to same index),
 * we create a (doubly) linked list in that index and store the data.
 * Inserts are very fast here. Worst case: O(n). Avg case: O(1)
 *
 * Open Addressing: Here the data is still stored in the hash table itself, but
 * when collision happens, we search for the next open slot and save the data
 * (along with key) there. Worst case: O(n). Avg case: O(1). In practice,
 * Chaining seems to be better than Open Addressing, if the table is big and
 * very small no. of collisions. Because, in Open Addressing, for every collision,
 * we may end up walking the table (to find an empty slot while inserting and
 * to search the key while searching) lots of times. I tried this in my experiment.
 * And for a specific type of input (from LRUCache question in leetcode), it
 * does 10 million unnecessary walks, whereas with chaining, it does only 4000
 * walks
 *
 * Perfect Hashing: When collision happens, we create another hash table (secondary)
 * at the index the collision is happening. In CLRS, it's stated that by carefully
 * selecting the hash functions H1 (for primary table) and H2 (for secondary table)
 * we can have O(1) worst case performance.  Worst case: O(1). Avg case: O(1)
 *
 */

 /**
  * For further efficiency, I've also used a simple bloom filter. Bloom filter
  * can only the absence of an element. If an element is absent, it will
  * definitely tell. But if an element is present, it may give false positives.
  * Used bloom filter per bucket.
  */

#define HT_BLOOM_BUCKET(key)  \
   ((key) % BLOOM_FILTER_BUCKETS_COUNT)

#define HT_BLOOM_TESTSET(bloom_bv, key)    \
   ((bloom_bv)[HT_BLOOM_BUCKET((key))] & (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE)))

#define HT_BLOOM_SET(bloom_bv, key)    \
   (bloom_bv)[HT_BLOOM_BUCKET((key))] |= (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE))


void ht_print_stats(HTHandle h)
{
   printf("HashTable: search_hits - %lu, search_misses - %lu,"
          " search_list_walks - %lu, bloom_filter_saves - %lu,"
          " searches - %lu, inserts - %lu\n",
          h->stats.search_hits, h->stats.search_misses,
          h->stats.search_list_walks, h->stats.bloom_filter_saves,
          h->stats.searches, h->stats.inserts);

   printf("Max collided elemetns in a row: %lu\n",
          find_max(h->stats.collisions, h->num_buckets));
}


 /**
  * if Keys match, it compares data also.
  */
void * ht_search_with_data_compare(HTHandle h_table, void *key, void *data)
{
   uint32_t hash_key = h_table->hash_fn(key);
   uint32_t bucketIndex = hash_key % h_table->num_buckets;
   ListNode *itr;
   HashItem *item;

   h_table->stats.searches++;

   if (!HT_BLOOM_TESTSET(h_table->buckets[bucketIndex].bloom_bv, hash_key)) {
      h_table->stats.bloom_filter_saves++;
      h_table->stats.search_misses++;
      return NULL;   // bloom filter says the key is not there.
   }

   LIST_FOR_ALL(&h_table->buckets[bucketIndex].list, itr) {
      item = (HashItem *) itr;
      if (h_table->compare_key_fn(item->key, key) == 0) {
         h_table->stats.search_hits++;

         if (!data) {
            return item->data;
         } else if (h_table->compare_data_fn(data, item->data) == 0) { // compare data
            return item->data;
         } else {
            return NULL;
         }
         break;
      }
      h_table->stats.search_list_walks++;
   }

   // key not found
   h_table->stats.search_misses++;
   return NULL;
}


void *ht_search(HTHandle h_table, void *key)
{
   return ht_search_with_data_compare(h_table, key, NULL);
}

ReturnStatus ht_insert(HTHandle h_table, void *key, void *data)
{
   uint32_t hash_key = h_table->hash_fn(key);
   uint32_t bucketIndex = hash_key % h_table->num_buckets;
   HashItem *item = NULL;

   h_table->stats.inserts++;

   if (h_table->buckets[bucketIndex].num_keys > 0) {
      h_table->stats.collisions[bucketIndex]++;
   }

   item = malloc(sizeof(HashItem));
   if (!item) {
      return NO_MEM;
   }

   list_init_element(&item->link, NULL);
   item->key = h_table->key_alloc_fn(key);
   if (!item->key) {
      free(item);
      return NO_MEM;
   }

   item->data = data;
   list_insert_at_front(&h_table->buckets[bucketIndex].list, &item->link);
   h_table->buckets[bucketIndex].num_keys++;

   HT_BLOOM_SET(h_table->buckets[bucketIndex].bloom_bv, hash_key);

   return OK;
}

void * ht_remove(HTHandle h_table, void *key, bool free_item)
{
   uint32_t hash_key = h_table->hash_fn(key);
   uint32_t bucketIndex = hash_key % h_table->num_buckets;
   HashItem *item = NULL;
   ListNode *itr = NULL;
   void *data = NULL;

   LIST_FOR_ALL(&h_table->buckets[bucketIndex].list, itr) {
      item = (HashItem *) itr;
      if (h_table->compare_key_fn(item->key, key) == 0) {
         list_item_unlink(&item->link);
         free(item->key);

         data = item->data;
         if (free_item) {
            free(data);
            data = NULL;
         }

         free(item);
         item = NULL;

         h_table->buckets[bucketIndex].num_keys--;
         break;
      }
   }

   return data;
}

void ht_print(HTHandle h_table)
{
   int i = 0;
   ListNode *itr;
   HashItem *item;

   for (i = 0; i < h_table->num_buckets; i++) {
      LIST_FOR_ALL(&h_table->buckets[i].list, itr) {
         item = (HashItem *) itr;
         printf("(");
         h_table->print_kv_fn(item->key, item->data);
         printf(") -> ");
      }
      printf(" ~~ \n");
   }

   printf("\n");
}

HTHandle ht_alloc(int num_buckets,                 // IN
                  HTHashFn hash_fn,                // IN
                  HTKeyAllocFn key_alloc_fn,       // IN
                  HTCompareKey compare_key_fn,     // IN
                  HTCompareData compare_data_fn,   // IN
                  HTPrintKV print_kv_fn)           // IN
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

   h_table->key_alloc_fn = key_alloc_fn;
   h_table->hash_fn = hash_fn;
   h_table->compare_key_fn = compare_key_fn;
   h_table->print_kv_fn = print_kv_fn;
   h_table->compare_data_fn = compare_data_fn;

   return h_table;

free_table:
   free(h_table);
   return NULL;
}

void ht_destroy(HTHandle h_table, bool destroy_data)
{
   int i;
   void *data;
   ListNode *itr;
   HashItem *item;

   for (i = 0; i < h_table->num_buckets; i++) {
      LIST_FOR_ALL(&h_table->buckets[i].list, itr) {
         item = (HashItem *) itr;
         data = ht_remove(h_table, item->key, destroy_data);
      }
   }

   free(h_table);
}

typedef struct NodeData {
   int value; // for debugging purpose
   bool visited;
} NodeData;

// space: O(n)
// time: O(n)
int longestConsecutive(int* nums, int numsSize) {
   HTHandle htable = ht_alloc(numsSize, ht_hash_clrs_int_fn, key_alloc_fn_int_fn,
                              compare_key_int_fn, NULL, NULL);
   int i = 0, longest_consec_len = 0, tmp_int = 0;
   ListNode *tmpListNode = NULL, *nextnode = NULL, *curnode = NULL;
   NodeData *tmpNodeData = NULL;

   for (i = 0; i < numsSize; i++) {
      tmpNodeData = (NodeData *) malloc(sizeof(NodeData));
      assert(tmpNodeData != NULL);

      tmpNodeData->value = nums[i];
      tmpNodeData->visited = false;

      tmpListNode = list_alloc_element(tmpNodeData);
      assert(tmpListNode != NULL);
      list_make_element_self_ref(tmpListNode);

      ht_insert(htable, &nums[i], tmpListNode);
   }

   for (i = 0; i < numsSize; i++) {
      curnode = ht_search(htable, &nums[i]);

      tmp_int = nums[i] + 1;
      nextnode = ht_search(htable, &tmp_int);

      if (nextnode != NULL) {
         list_items_link(curnode, nextnode);
      }
   }

   for (i = 0; i < numsSize; i++) {
      tmp_int = 0;
      curnode = ht_search(htable, &nums[i]);
      tmpNodeData = (NodeData *) curnode->data;

      // Since we'll not proceed the chain of a node which is visited already,
      // the entire fn is O(n)
      while(!tmpNodeData->visited) {
         tmpNodeData->visited = true;
         tmp_int++;
         curnode = curnode->next;
         tmpNodeData = (NodeData *) curnode->data;
      }

      if (tmp_int > longest_consec_len) {
         longest_consec_len = tmp_int;
      }
   }

   return longest_consec_len;
}
