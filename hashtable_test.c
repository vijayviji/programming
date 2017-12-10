#include "hashtable.h"

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
      if(ht_insert(h_table, keys[i], data) != OK) {
         printf("Insert failed.\n");
         return 1;
      }
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
   ht_print(h_table);
   ht_print_stats(h_table);
   ht_destroy(h_table, true);
}
