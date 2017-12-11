#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "../common.h"

/**
 * Find next one which is "just" taller than current one. If there's nothing
 * then, find the one that's closest to current one's height.
 */
uint32_t find_next_best(uint32_t *arr, uint32_t start, uint32_t n)
{
   int i = 0, best = start + 1;

   for (i = start + 1; i < n; i++) {
      if (arr[i] > arr[start]) {
         best = i;
         break;
      }

      if (arr[i] > arr[best]) {
         best = i;
      }
   }

   return best;
}

/**
 * Step 1: Compute the distance between to heights and imagine that there's empty
 * space between the two. And calculate the boxes it can hold.
 *
 * Step 2: Now, loop through the heights between the two given heights and subtract
 * their height from the boxes calculated in previous step.
 *
 */
uint32_t calculate_boxes(uint32_t *arr, uint32_t start, uint32_t end)
{
   uint32_t i, distance, total_boxes, boxes_to_subtract = 0, min;

   // step 1
   distance = (end - start - 1);
   min = MIN(arr[start], arr[end]);
   total_boxes = min * distance;

   // step 2
   for (i = start + 1; i < end; i++) {
      boxes_to_subtract += MIN(min, arr[i]);
   }

   total_boxes -= boxes_to_subtract;

   return total_boxes;
}

/**
 * Running time analysis:
 *       O(n)
 *    Basically we go through the numbers in the array, only once.
 *    Both the loop and find_next_best() are pushing forward, never
 *    traversing the same number again. Note: we shift i, after
 *    finding out j. If we don't, then this will be O(n^2).
 */
uint32_t trap(uint32_t *arr, uint32_t n)
{
   uint32_t i, j, total_boxes = 0, tmp;

   for (i = 0; i < n; i++) {
      j = find_next_best(arr, i, n);
      tmp = calculate_boxes(arr, i, j);

      printf("%d - %d: %d boxes\n", i, j, tmp);
      total_boxes += tmp;
      i = j - 1; // i++ in the for loop will make i = j in the next iteration.
   }

   return total_boxes;
}

// below for testing
int main()
{
   uint32_t a[] = {0,1,0,2,1,0,1,3,2,1,2,1};

   printf("%u\n", trap(a, 12));
}
// Some test cases

//    |    |
//    |    ||
// || |  | ||
// || |  | ||
// || |  | ||
// || |  ||||
// ||||||||||

//
//        |
//    |   || |
//  | || ||||||

// |
// | ||
// | ||
// |||| |
// ||||||
// ||||||
// |||||||
// |||||||
// |||||||
