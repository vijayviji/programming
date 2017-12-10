/**
 *
 * In this program malloc is not allowed. Hence used a global variable.
 *
 */



#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

/**
 * Bits start from 0 to (num_bits - 1) in map
 */
typedef struct BMAP {
   uint32_t num_bits;
   uint32_t *map;
} BMAP;




BMAP BITMAP;
uint32_t MAP[10];



typedef BMAP * BMAPHandle;

#define BITS_IN_UINT64 32

bool bm_isset(BMAPHandle bmap, uint32_t num)
{
   uint32_t bucket, entry;

   bucket = num / BITS_IN_UINT64;
   entry = num % BITS_IN_UINT64;

   if (num >= bmap->num_bits) {
      return false;
   }

   return bmap->map[bucket] & (1 << entry);
}

void bm_set(BMAPHandle bmap, uint32_t num)
{
   uint32_t bucket, entry;

   bucket = num / BITS_IN_UINT64;
   entry = num % BITS_IN_UINT64;

   if (num < bmap->num_bits) {
      bmap->map[bucket] |= (1 << entry);
   }
}

void bm_unset(BMAPHandle bmap, uint32_t num)
{
   uint32_t bucket, entry;

   bucket = num / BITS_IN_UINT64;
   entry = num % BITS_IN_UINT64;

   if (num < bmap->num_bits) {
      bmap->map[bucket] &= ~(1 << entry);
   }
}

BMAPHandle bm_alloc(uint32_t num_bits)
{
   BMAPHandle bmap = NULL;
   uint32_t size = (num_bits / sizeof(uint32_t));

   if (!size) {
      if (num_bits) {
         size = sizeof(uint32_t);
      } else {
         return bmap;
      }
   }

   bmap = &BITMAP;
   bmap->num_bits = num_bits;
   bmap->map = MAP;
   memset(bmap->map, 0, size);

   return bmap;
}

int lengthOfLongestSubstring(char* s) {
   BMAPHandle bmap = bm_alloc(300);
   int start = 0, end = 1, len = 1, i, max_len = 1, max_start = 0, max_end = 1;

   if (s == NULL || s[0] == '\0') {
      return 0;
   }

   bm_set(bmap, s[0]);

   while(s[end] != '\0') {
      if (!bm_isset(bmap, s[end])) {
         bm_set(bmap, s[end]);
         end++;
         len++;

         if (max_len < len) {
            max_len = len;
            max_start = start;
            max_end = end;
         }
      } else {
         assert(start < end);
         bm_unset(bmap, s[start]);
         start++;
         len--;
      }
   }

   s[max_end] = '\0';
   printf("%s\n", &s[max_start]);

   return max_len;
}

/**
 * BELOW testing
 */

int main()
{
   char s[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ abcdefghijklmn";
   char *str = malloc(strlen(s) + 1);

   memcpy(str, s, strlen(s) + 1);
   printf("%d\n", lengthOfLongestSubstring(str));
}
