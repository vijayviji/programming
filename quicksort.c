#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "quicksort.h"


uint64_t qs_select(void *arr,              // IN: array of objects to be sorted.
                   uint64_t p,             // IN: start position.
                   uint64_t q,             // IN: end position.
                   QSComparator qc_fn,     // IN: comparator function
                   uint32_t item_size,     // IN: size of each object in arr.
                   void *scratch_buffer)   // IN: scratch buffer of item_size
                                           //     size to use for temp purpose.
{
   uint64_t i = p, j;
   void *r = arr + (q * item_size);

   for (j = p; j < q; j++) {
      if (qc_fn(arr + (j * item_size), r) < 0) {
         // swap arr[i] and arr[j]
         memcpy(scratch_buffer, arr + (i * item_size), item_size);
         memcpy(arr + (i * item_size), arr + (j * item_size), item_size);
         memcpy(arr + (j * item_size), scratch_buffer, item_size);
         i++;
      }
   }

   // swap arr[i] and arr[q]
   memcpy(scratch_buffer, arr + (i * item_size), item_size);
   memcpy(arr + (i * item_size), arr + (q * item_size), item_size);
   memcpy(arr + (q * item_size), scratch_buffer, item_size);

   return i;
}

void quicksort(void *arr,           // IN: array to be sorted
               uint64_t p,          // IN: starting index in arr
               uint64_t q,          // IN: ending index in arr
               QSComparator qc_fn,  // IN: comparator fn
               uint32_t item_size,  // IN: size of an object in arr
               void *scratch_buffer)// IN: scratch buffer of item_size
                                       //  size to use for temp purpose.
{
   uint64_t r;

   if (p >= q) {
      return;
   }

   r = qs_select(arr, p, q, qc_fn, item_size, scratch_buffer);

   // because we're using unsinged int, if r = 0, then r - 1 will screw up.
   if (r != 0) {
      quicksort(arr, p, r - 1, qc_fn, item_size, scratch_buffer);
   }
   //printf("r: %d, p:%d, q:%d\n", r, p, q);

   quicksort(arr, r + 1, q, qc_fn, item_size, scratch_buffer);
}
