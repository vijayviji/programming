#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

/**
 * Bits start from 0 to (num_bits - 1) in map
 */
typedef struct BMAP {
   uint32_t num_bits;
   uint32_t *map;
} BMAP;

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

   bmap = (BMAPHandle) malloc(sizeof(BMAPHandle));
   bmap->num_bits = num_bits;
   bmap->map = (uint32_t *) malloc(size);
   memset(bmap->map, 0, size);

   return bmap;
}

/**
 * Following is test
 */

int main()
{
   BMAPHandle bmap = bm_alloc(1000);
   uint64_t bit = 120;

   printf("%u\n", bit);

   printf("%d\n", bm_isset(bmap, bit));
   bm_set(bmap, bit);
   printf("%d\n", bm_isset(bmap, bit));
   bm_unset(bmap, bit);
   printf("%d\n", bm_isset(bmap, bit));

}
