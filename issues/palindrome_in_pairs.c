#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../common.h"
#include "../hashtable.h"

void add_to_result(int **return_arr, int i, int j, int *columnSizes, int *k)
{
   return_arr[*k] = (int *) malloc(sizeof(int) * 2);
   assert(return_arr[*k] != NULL);

   return_arr[*k][0] = i;
   return_arr[*k][1] = j;
   columnSizes[*k] = 2; // 2 elements in this set. this is stupid requirement.
   (*k)++;
}

#define MAX_RESULT 10000

int** palindromePairs(char** words, int wordsSize, int** columnSizes, int* returnSize)
{
   bool str_reveresed = false;
   int i, j = 0, l, k = 0;
   char *str_copy = NULL;
   int **return_arr, empty_string_idx = -1, *retColumnSizes;
   HTHandle h_table = ht_alloc(wordsSize, sdbm_str_fn, key_alloc_fn_str,
                               compare_key_str_fn, compare_data_int_fn,
                               print_kv_int_fn);
   int *tmp = NULL;

   assert(returnSize != NULL);
   *returnSize = 0;

   assert(h_table != NULL);

   return_arr = (int **) malloc(sizeof(int *) * MAX_RESULT);
   assert(return_arr != NULL);

   retColumnSizes = (int *) malloc(sizeof(int) * MAX_RESULT);
   assert(retColumnSizes != NULL);

   for (i = 0; i < wordsSize; i++) {
      tmp = (int *) malloc(sizeof(int));
      assert(tmp != NULL);

      *tmp = i;
      if (words[i][0] == '\0') {
         empty_string_idx = i;
      } else {
         ht_insert(h_table, words[i], tmp);
      }
   }

doAgain:
   for (i = 0; i < wordsSize; i++) {
       //printf("%d - ", i);
      if (str_reveresed) {
         str_reverse(words[i]); // in-place reverse
      }
      l = strlen(words[i]);

      if (!str_reveresed && empty_string_idx != -1 && i != empty_string_idx
          && is_palindrome(words[i])) {
         add_to_result(return_arr, i, empty_string_idx, retColumnSizes, &k);

         add_to_result(return_arr, empty_string_idx, i, retColumnSizes, &k);

      }

      for (j = l - 1; j >= 0; j--) {
         if (j == 0 && str_reveresed) {
            continue;
         }
         if (!str_reveresed) {
            str_copy = str_reverse_cpy(&words[i][j]);
            tmp = (int *) ht_search(h_table, str_copy);
            free(str_copy);
         } else {
            tmp = (int *) ht_search(h_table, &words[i][j]);
         }

         if (tmp) {

            assert(*tmp < wordsSize);

            if (j != 0) {
                if (!is_palindrome_ij(words[i], 0, j - 1)) {
                   continue;
                }
            }

            if (i == *tmp) {
                assert(j == 0);
               continue;
            }
            if (str_reveresed) {
               add_to_result(return_arr, i, *tmp, retColumnSizes, &k);
            } else {
               add_to_result(return_arr, *tmp, i, retColumnSizes, &k);
            }
         }
      }
   }

   if (!str_reveresed) {
      str_reveresed = true;
      goto doAgain;
   }

   *returnSize = k;
   *columnSizes = retColumnSizes;

   ht_destroy(h_table, true);
   return return_arr;
}

int main()
{
#define MAX_STR 4
   char **words = malloc(MAX_STR * sizeof(char *));
   char s[][200] = {"ab","ba","abc","cba"};
   int i, returnSize, *columnSizes = NULL, **returnArr;

   for (i = 0; i < MAX_STR; i++) {
      words[i] = malloc(200);
      strcpy(words[i], s[i]);
      printf("%s, ", s[i]);
   }
   printf("\n");

   returnArr = palindromePairs(words, MAX_STR, &columnSizes, &returnSize);
   for (i = 0; i < returnSize; i++) {
      printf("[%d, %d] ", returnArr[i][0], returnArr[i][1]);
   }
   printf("\n");

   return 0;
}
