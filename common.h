#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <malloc.h>

typedef enum ReturnStatus {
   OK = 0,
   FAIL = 1,
   NO_MEM = 2
} ReturnStatus;

#define MAX(a, b) (((a) > (b))? (a): (b))
#define MIN(a, b) (((a) < (b))? (a): (b))

static uint64_t find_max(uint64_t *arr, uint64_t count)
{
   uint64_t i;
   uint64_t max = 0;

   for (i = 0; i < count; i++) {
      if (arr[i] > max) {
         max = arr[i];
      }
   }

   return max;
}

char * substring(char source[], int start, int len) {
   int i = start;

   char *target = malloc(len);
   if (!target) {
      return NULL;
   }

   while (i < len) {
      target[i] = source[i];
      i++;
   }
   target[i] = '\0';

   return target;
}

// in-place reverse.
char * str_reverse(char *str)
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

#endif // _COMMON_H
