#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct DataItem {
   void *data;
   int key;
} DataItem;


typedef void (*HTPrintItem) (void *);
typedef bool (*HTCompareData) (void *, void *);
typedef struct HashTable {
   int num_keys;
   DataItem *table;

   HTCompareData compare_data;
   HTPrintItem print_item;
} HashTable;

typedef HashTable * HTHandle;


int hashCode(HTHandle h_table, int key) {
   return key % h_table->num_keys;
}

/**
 * if Keys match, it compares data also.
 */
void *ht_search_with_data_compare(HTHandle h_table, int key, void *data)
{
   //get the hash
   int hashIndex = hashCode(h_table, key), originalHashIndex;
   originalHashIndex = hashIndex;

   //move in array until an empty
   while(h_table->table[hashIndex].key != -1) {

      if(h_table->table[hashIndex].key == key) {

         if (data == NULL) {
            return h_table->table[hashIndex].data;
         } else if (h_table->compare_data(data,
                     h_table->table[hashIndex].data)) {
            return h_table->table[hashIndex].data;
         }
      }

      //go to next cell
      ++hashIndex;

      // we searched entire table
      if (hashIndex == originalHashIndex) {
         break;
      }

      //wrap around the table
      hashIndex %= h_table->num_keys;
   }

   return NULL;
}


void *ht_search(HTHandle h_table, int key) {
   return ht_search_with_data_compare(h_table, key, NULL);
}

void ht_insert(HTHandle h_table, int key, void *data) {

   //get the hash
   int hashIndex = hashCode(h_table, key), originalHashIndex;
   originalHashIndex = hashIndex;

   //move in array until an empty or deleted cell
   while(h_table->table[hashIndex].key != -1) {
      //go to next cell
      ++hashIndex;

      // we searched entire table
      if (hashIndex == originalHashIndex) {
         printf("TABLE IS FULL\n");
         break;
      }

      //wrap around the table
      hashIndex %= h_table->num_keys;
   }

   h_table->table[hashIndex].key = key;
   h_table->table[hashIndex].data = data;
}

void ht_delete(HTHandle h_table, int key) {
   //get the hash
   int hashIndex = hashCode(h_table, key);

   //move in array until an empty
   while(h_table->table[hashIndex].key != -1) {

      if(h_table->table[hashIndex].key == key) {
         h_table->table[hashIndex].key = -1;
         h_table->table[hashIndex].data = NULL;
      }

      //go to next cell
      ++hashIndex;

      //wrap around the table
      hashIndex %= h_table->num_keys;
   }
}

void ht_display(HTHandle h_table) {
   int i = 0;

   for(i = 0; i < h_table->num_keys; i++) {
      if(h_table->table[i].key != -1) {
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

   h_table->num_keys = num_keys;
   h_table->table = (DataItem *) malloc(sizeof(DataItem) * num_keys);
   h_table->print_item = print_item;
   h_table->compare_data = compare_data;

   for (i = 0; i < num_keys; i++) {
      h_table->table[i].key = -1;
      h_table->table[i].data = NULL;
   }

   return h_table;
}

/**
 * if data doesn't match, it will return true. This is specific for this
 * program when duplicates are there in the hash TABLE
 */
bool compare_opposite_data(void *data1, void *data2)
{
   int val1 = *(int *) data1, val2 = *(int *) data2;

   return val1 != val2;
}

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int* twoSum(int* nums, int numsSize, int target) {
    int i, j = numsSize - 1, *retArr;

    // (key, value) = (num[i], i)
    HTHandle h_table = ht_alloc(numsSize, NULL, compare_opposite_data);
    int *tmp;

    retArr = malloc(sizeof(int) * 2);

   for (i = 0; i <= j; i++) {
      tmp = (int *) ht_search_with_data_compare(h_table, target - nums[i], &i);

      if (tmp != NULL) {
         retArr[0] = i;
         retArr[1] = *tmp;
         break;
      }

      tmp = (int *) malloc(sizeof(int));
      *tmp = i;
      ht_insert(h_table, nums[i], tmp);
   }

    return retArr;
}


int main()
{
   int a[3] = {3, 4, 5}, *res;


   res = twoSum(a, 3, 9);
   printf("%d %d\n", res[0], res[1]);

}
