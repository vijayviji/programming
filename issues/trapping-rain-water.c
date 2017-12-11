#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "../common.h"

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

uint32_t calculate_boxes(uint32_t *arr, uint32_t start, uint32_t end)
{
   uint32_t i, distance, total_boxes, boxes_to_subtract = 0, min;

    distance = (end - start - 1);
    min = MIN(arr[start], arr[end]);
    total_boxes = min * distance;

   for (i = start + 1; i < end; i++) {
      boxes_to_subtract += MIN(min, arr[i]);
   }

   total_boxes -= boxes_to_subtract;

   return total_boxes;
}

uint32_t trap(uint32_t *arr, uint32_t n)
{
   uint32_t i, j, total_boxes = 0, tmp;

   for (i = 0; i < n; i++) {
      j = find_next_best(arr, i, n);
      tmp = calculate_boxes(arr, i, j);
      printf("%d - %d: %d boxes\n", i, j, tmp);
      total_boxes += tmp;
      i = j - 1;
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
