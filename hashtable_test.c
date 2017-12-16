#include "hashtable.h"

/*
 * Below are for testing
 */

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
#define KEY_SIZE 10
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


#define MAX 1000
int main()
{
   HTHandle h_table = ht_alloc(MAX, sdbm_str, key_alloc_fn_str,
                               compare_key_str, compare_data_str,
                               print_kv_str);
   int i;
   char *key = "1111111111", *keys[MAX], *data;

   srand(time(NULL));   // should only be called once

   for (i = 0; i < MAX; i++) {
      if (i == 0) {
         keys[i] = next_key(key);
      } else {
         keys[i] = next_key(keys[i - 1]);
      }
      assert(keys[i] != NULL);

      data = malloc(25);
      snprintf(data, 25, "%s - world - %d", keys[i], i);
      printf("Inserting (%s, %s)\n", keys[i], data);
      if(ht_insert(h_table, keys[i], data) != OK) {
         printf("Insert failed.\n");
         return 1;
      }
   }

   for (i = 0; i < MAX; i++) {
      data = (char *) ht_search(h_table, keys[i]);
      if (!data) {
         printf("search: %s. result: NULL\n", keys[i]);
      } else {
         printf("search: %s. result: %s\n", keys[i], data);
      }
   }

   printf("\nSearching for keys that are not there:\n");

   for (i = MAX; i < MAX+100; i++) {
      data = (char *) ht_search(h_table, "adsfadsf");
      if (!data) {
         printf("search: %s. result: NULL\n", "adsfadsf");
      } else {
         printf("search: %s. result: %s\n", "adsfadsf", data);
      }
   }

   data = ht_remove(h_table, &keys[i - 1], false);
   free(data);
   ht_print(h_table);
   ht_print_stats(h_table);
   ht_destroy(h_table, true);
}
