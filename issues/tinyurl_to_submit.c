typedef enum ReturnStatus {
   OK = 0,
   FAIL = 1,
   NO_MEM = 2
} ReturnStatus;

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

#define HT_BLOOM_BUCKET(key)  \
   ((key) % BLOOM_FILTER_BUCKETS_COUNT)

#define HT_BLOOM_TESTSET(bloom_bv, key)    \
   ((bloom_bv)[HT_BLOOM_BUCKET((key))] & (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE)))

#define HT_BLOOM_SET(bloom_bv, key)    \
   (bloom_bv)[HT_BLOOM_BUCKET((key))] |= (1 << ((key) % BLOOM_FILTER_VECTOR_SIZE))


void ht_print_stats(HTHandle h)
{
   printf("HashTable: search_hits - %u, search_misses - %u,"
          " search_list_walks - %u, bloom_filter_saves - %u,"
          " searches - %u, inserts - %u\n",
          h->stats.search_hits, h->stats.search_misses,
          h->stats.search_list_walks, h->stats.bloom_filter_saves,
          h->stats.searches, h->stats.inserts);

   printf("Max collided elemetns in a row: %d\n",
          find_max(h->stats.collisions, h->num_buckets));
}


 /**
  * if Keys match, it compares data also.
  */
void * ht_search_with_data_compare(HTHandle h_table, void *key, void *data)
{
   uint32_t hash_key = h_table->hash_fn(key);
   uint32_t bucketIndex = hash_key % h_table->num_buckets;
   ListNodeCustom *itr;
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
   ListNodeCustom *itr = NULL;
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
   ListNodeCustom *itr;
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






// key and data are string

uint32_t sdbm_str(void *key)
{
   return sdbm(key);
}

void * key_alloc_fn_str(void *key)
{
   char *strkey = (char *) key;
   uint32_t len = strlen(strkey);
   char *new_key = malloc(len + 1);

   if (new_key) {
      memcpy(new_key, strkey, len + 1);
   }

   return new_key;
}

int compare_key_str(void *key1, void *key2)
{
   char *strkey1 = (char *) key1, *strkey2 = (char *) key2;

   return strcmp(strkey1, strkey2);
}

int compare_data_str(void *data1, void *data2)
{
   char *val1 = (char *) data1, *val2 = (char *) data2;

   return strcmp(val1, val2);
}

void print_kv_str(void *key, void *data)
{
   char *str_key = (char *) key;
   char *str_data = (char *) data;

   printf("%s, %s", str_key, str_data);
}

char add_char(char s, int val, int *carry)
{
#define MAX_EDGES 4
   int i;
   char edges[MAX_EDGES][2] = {{'0', '9'}, {'A', 'Z'}, {'a', 'z'},
                               {'\0', '\0'}};

   *carry = 0;

   for (i = 0; i < MAX_EDGES - 1; i++) {
      if (s >= edges[i][0] && s <= edges[i][1]) {
         s = s + val;
         if (s > edges[i][1]) {
            if (edges[i + 1][0] != '\0') {
               val = s - edges[i][1] - 1;
               s = edges[i + 1][0];
            } else {
               *carry = s - edges[i][1];
               s = edges[0][0];
               break;
            }
         } else {
            val = 0;
            carry = 0;
         }
      }
   }

   return s;
}

/*
 * next short code
 */
#define KEY_SIZE 6
char * next_key(char *prev_key)
{
   int i, carry = 1;
   char *next_key = malloc(KEY_SIZE + 1); // +1 for '\0'

   if (!next_key) {
      return NULL;
   }
   memset(next_key, 0, KEY_SIZE + 1);

   assert(strlen(prev_key) == KEY_SIZE);

   for (i = KEY_SIZE - 1; i >= 0; i--) {
      next_key[i] = add_char(prev_key[i], carry, &carry);

      if (i == 0) {
         assert(carry == 0);
      }
   }
   return next_key;
}

char last_key[] = "111111";
HTHandle h_table = NULL;

#define TINYURL "http://tinyurl.com/"
#define TINYURL_LEN 19

/** Encodes a URL to a shortened URL. */
char* encode(char* longUrl) {
   char *key;
   char *tinyurl = malloc(TINYURL_LEN + KEY_SIZE + 1);

   if (!h_table) {
      h_table = ht_alloc(1000, sdbm_str, key_alloc_fn_str,
                         compare_key_str, compare_data_str,
                         print_kv_str);
   }

   if (!tinyurl) {
      return NULL;
   }
   strcpy(tinyurl, TINYURL);

   key = next_key(last_key);
   if (!key) {
      return NULL;
   }

   if (ht_insert(h_table, key, longUrl) != OK) {
      return NULL;
   }

   return strcat(tinyurl, key);
}

/** Decodes a shortened URL to its original URL. */
char* decode(char* shortUrl) {
   char *key, *tmp;

   if (!h_table) {
      return NULL;
   }

   tmp = strstr(shortUrl, TINYURL);
   if (!tmp) {
      return NULL;
   }

   key = tmp + TINYURL_LEN;
   return ht_search(h_table, key);
}
