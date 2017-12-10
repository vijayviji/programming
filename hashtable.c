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
   ListNode *itr;
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
   ListNode *itr = NULL;
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
   ListNode *itr;
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
