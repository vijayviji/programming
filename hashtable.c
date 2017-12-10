#include "hashtable.h"

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
