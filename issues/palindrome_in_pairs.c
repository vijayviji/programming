#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../common.h"

typedef struct Word {
   char *word;
   int index;
   bool is_reversed;
   bool is_palindrome;
   bool is_empty_string;
} Word;

typedef int (*QSComparator)(void *item1, void *item2);


int qs_select(void *arr, int p, int q, QSComparator qc_fn, uint32_t itemSize, void *buffer)
{
   int i = p - 1, j;
   void *r = arr + (q * itemSize);
   //void *tmp = malloc(itemSize);

   // if (!tmp) {
   //    return -10;
   // }

   for (j = p; j < q; j++) {
      if (qc_fn(arr + (j * itemSize), r) < 0) {
         i++;
         memcpy(buffer, arr + (i * itemSize), itemSize);
         memcpy(arr + (i * itemSize), arr + (j * itemSize), itemSize);
         memcpy(arr + (j * itemSize), buffer, itemSize);
      }
   }
   i++;
   memcpy(buffer, arr + (i * itemSize), itemSize);
   memcpy(arr + (i * itemSize), arr + (q * itemSize), itemSize);
   memcpy(arr + (q * itemSize), buffer, itemSize);

   //free(buffer);
   return i;
}

void quicksort(void *arr, int p, int q, QSComparator qc_fn, uint32_t itemSize, void *buffer)
{
   int r;

   if (p >= q) {
      return;
   }

   r = qs_select(arr, p, q, qc_fn, itemSize, buffer);
   //printf("r: %d, p:%d, q:%d\n", r, p, q);

   if (r == -10) {
      printf("memalloc failed");
      return;
   }

   quicksort(arr, p, r - 1, qc_fn, itemSize, buffer);
   quicksort(arr, r + 1, q, qc_fn, itemSize, buffer);
}

void * reverse(char *str)
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

int qs_comparator_str(void *str1, void *str2)
{
   return strcmp(str1, str2);
}

void word_destroy(Word *words, int wordscount)
{
   int j;

   for (j = 0; j < wordscount * 2; j++) {
      if (words[j].word) {
         free(words[j].word);
      }
   }

   free(words);
}

bool is_palindrome(char *s)
{
   int l = strlen(s), i;

   for (i = 0; i < l / 2; i++) {
      if (s[i] != s[l - i - 1]) {
          return false;
      }
   }
   return true;
}

Word * word_alloc_and_init(char **words, int wordscount, int *empty_string_encountered)
{
   int i, j, l;
   Word *new_words = malloc(2 * wordscount * sizeof(Word));

   if (!new_words) {
      return NULL;
   }
   memset(new_words, 0, 2 * wordscount * sizeof(Word));

   for (i = 0; i < wordscount; i++) {
      l = strlen(words[i]);

      new_words[i * 2].word = malloc(l + 1);
      if (!new_words[i].word) {
         goto free_words;
      }

      new_words[i * 2].index = i;
      strcpy(new_words[i * 2].word, words[i]);
      new_words[i * 2].is_reversed = false;
      new_words[i * 2].is_palindrome = is_palindrome(new_words[i * 2].word);
      new_words[i * 2].is_empty_string = (new_words[i * 2].word[0] == '\0');

      // checking only for forward string is fine
      if (new_words[i * 2].is_empty_string) {
         *empty_string_encountered = i;
      }

      new_words[i * 2 + 1].word = malloc(l + 1);
      if (!new_words[i].word) {
         goto free_words;
      }

      new_words[i * 2 + 1].index = i;
      strcpy(new_words[i * 2 + 1].word, reverse(words[i]));
      new_words[i * 2 + 1].is_reversed = true;
      new_words[i * 2 + 1].is_palindrome = is_palindrome(new_words[i * 2 + 1].word);
      new_words[i * 2 + 1].is_empty_string = (new_words[i * 2 + 1].word[0] == '\0');
   }

   return new_words;

free_words:
   word_destroy(new_words, wordscount);
   return NULL;
}

int qs_comparator_word(void *word1, void *word2)
{
   Word *w1 = word1, *w2 = word2;
   char *s1 = w1->word, *s2 = w2->word;

   return strcmp(s1, s2);
}

int palidrome_equality(Word *word1, Word *word2)
{
   Word *w1 = word1, *w2 = word2;
   char *s1 = w1->word, *s2 = w2->word;

   if (s1 == NULL && s2) {
      return -1 * (*s2);
   }

   if (s2 == NULL && s1) {
      return *s1;
   }

   if (*s1 == '\0' && *s2 == '\0') {
      return 0;
   } else if (*s1 == '\0' || *s2 == '\0') {
       return 1;
   }

   while (*s1 != '\0' && *s2 != '\0') {
      if (*s1 != *s2) {
         return *s1 - *s2;
      }
      s1++;
      s2++;
   }
   return 0;
}

int** palindromePairs(char** words, int wordsSize, int** columnSizes, int* returnSize)
{
   int i, j = 0;
   Word *new_words, buffer;
   int return_arr[3000][2], **real_return_arr;
   int empty_string_idx = -1;

   *returnSize = 0;

   new_words = word_alloc_and_init(words, wordsSize, &empty_string_idx);
   if (!new_words) {
      return NULL;
   }

   quicksort(new_words, 0, wordsSize * 2 - 1, qs_comparator_word, sizeof(Word), &buffer);

   for (i = 0;  i < 2 * wordsSize; i++) {
      if (!palidrome_equality(&new_words[i], &new_words[i + 1]) &&
          new_words[i].index != new_words[i + 1].index) {
         (*returnSize)++;
         if (new_words[i].is_reversed) {
            return_arr[j][0] = new_words[i + 1].index;
            return_arr[j][1] = new_words[i].index;
            printf("[%d, %d]\n", new_words[i + 1].index, new_words[i].index);
         } else {
            return_arr[j][1] = new_words[i + 1].index;
            return_arr[j][0] = new_words[i].index;
            printf("[%d, %d]\n", new_words[i].index, new_words[i + 1].index);
         }
         j++;
      }

      if (empty_string_idx != -1 && !new_words[i].is_empty_string &&
          new_words[i].is_palindrome) {
          (*returnSize)++;
          if (new_words[i].is_reversed) {
             return_arr[j][0] = new_words[i].index;
             return_arr[j][1] = empty_string_idx;
             printf("[%d, %d]\n", new_words[i].index, empty_string_idx);
          } else {
             return_arr[j][0] = empty_string_idx;
             return_arr[j][1] = new_words[i].index;
             printf("[%d, %d]\n", empty_string_idx, new_words[i].index);
          }
         j++;
      }
   }

   real_return_arr = malloc(sizeof(int *) * (*returnSize));
   *columnSizes = malloc(sizeof(int) * (*returnSize));


   for (i = 0; i < *returnSize; i++) {
      real_return_arr[i] = malloc(sizeof(int) * 2);

      (*columnSizes)[i] = 2;
      real_return_arr[i][0] = return_arr[i][0];
      real_return_arr[i][1] = return_arr[i][1];
   }

   word_destroy(new_words, wordsSize);
   return real_return_arr;
}

// rev, ver, v
int qs_comparator_int(void *a, void *b)
{
   int a1 = *(int *) a, b1 = *(int *)b;

   return a1 - b1;
}

int main()
{
#define MAX_STR 6
   char **words = malloc(MAX_STR * sizeof(char *));
   char s[][200] = {"a","b","c","ab","ac","aa"};
   //int ints[MAX_STR] = {31, 22, 53, 14, 65, 96, 74, 38, 79, 10};
   int i, returnSize, *columnSizes = NULL;

   for (i = 0; i < MAX_STR; i++) {
      words[i] = malloc(200);
      strcpy(words[i], s[i]);
      //snprintf(words[i], 10, "asdfaf%d", i);
      //printf("%p - %s\n", words[i], words[i]);
   }

   palindromePairs(words, MAX_STR, &columnSizes, &returnSize);
   //quicksort(s, 0, 4, qs_comparator_str, MAX_STR);

   return 0;
}