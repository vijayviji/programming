#ifndef _QUICKSORT_H
#define _QUICKSORT_H

#include <stdint.h>

typedef int (*QSComparator)(void *item1, void *item2);

static int qs_comparator_str(void *str1, void *str2)
{
   return strcmp(str1, str2);
}

static int qs_comparator_int(void *a, void *b)
{
   int a1 = *(int *) a, b1 = *(int *)b;

   return a1 - b1;
}

void quicksort(void *arr, uint64_t p, uint64_t q, QSComparator qc_fn,
               uint32_t item_size, void *scratch_buffer);

#endif   // _QUICKSORT_H
