#include <string.h>
#include <malloc.h>
#include "quicksort.h"

int main()
{
#define STRINGS_COUNT 10
#define STRING_MAX_LEN 200 // this includes '\0' at end of string.
   char str[STRINGS_COUNT][STRING_MAX_LEN] = {"34a","bd","ac","zab","ac","qaa",
                                              "af3", "hjtdfgj", "ai9y23",
                                              "k;irt"};
   char scratch_buffer[STRING_MAX_LEN];
   int i;

   printf("Before sorting:\n");
   for (i = 0; i < STRINGS_COUNT; i++) {
      printf("%s\n", str[i]);
   }

   quicksort(str, 0, STRINGS_COUNT - 1, qs_comparator_str, STRING_MAX_LEN,
             &scratch_buffer);

   printf("\nAfter sorting:\n");
   for (i = 0; i < STRINGS_COUNT; i++) {
      printf("%s\n", str[i]);
   }

   return 0;
}

//
// int main()
// {
// #define MAX_INTS 10
//    int nums[MAX_INTS] = {8, 9, 4, 5, 3, 4, 2, 1, 0, 10}, i, scratch_buffer;
//
//    quicksort(nums, 0, MAX_INTS - 1, qs_comparator_int, sizeof(int),
//              &scratch_buffer);
//
//    for (i = 0; i < MAX_INTS; i++) {
//       printf("%d ", nums[i]);
//    }
//
//    printf("\n");
//
//    return 0;
// }
