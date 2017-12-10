#include "../hashtable.h"


// key and data are string

uint32_t sdbm_str(void *key)
{
   return sdbm(key);
}

void * key_alloc_fn_str(void *key)
{
   char *strkey = (char *) key;
   uint32_t len = strlen(strkey);
   char *new_key = malloc(len + 1);

   if (new_key) {
      memcpy(new_key, strkey, len + 1);
   }

   return new_key;
}

int compare_key_str(void *key1, void *key2)
{
   char *strkey1 = (char *) key1, *strkey2 = (char *) key2;

   return strcmp(strkey1, strkey2);
}

int compare_data_str(void *data1, void *data2)
{
   char *val1 = (char *) data1, *val2 = (char *) data2;

   return strcmp(val1, val2);
}

void print_kv_str(void *key, void *data)
{
   char *str_key = (char *) key;
   char *str_data = (char *) data;

   printf("%s, %s", str_key, str_data);
}

char add_char(char s, int val, int *carry)
{
#define MAX_EDGES 4
   int i;
   char edges[MAX_EDGES][2] = {{'0', '9'}, {'A', 'Z'}, {'a', 'z'},
                               {'\0', '\0'}};

   *carry = 0;

   for (i = 0; i < MAX_EDGES - 1; i++) {
      if (s >= edges[i][0] && s <= edges[i][1]) {
         s = s + val;
         if (s > edges[i][1]) {
            if (edges[i + 1][0] != '\0') {
               val = s - edges[i][1] - 1;
               s = edges[i + 1][0];
            } else {
               *carry = s - edges[i][1];
               s = edges[0][0];
               break;
            }
         } else {
            val = 0;
            carry = 0;
         }
      }
   }

   return s;
}

/*
 * next short code
 */
#define KEY_SIZE 6
char * next_key(char *prev_key)
{
   int i, carry = 1;
   char *next_key = malloc(KEY_SIZE + 1); // +1 for '\0'

   if (!next_key) {
      return NULL;
   }
   memset(next_key, 0, KEY_SIZE + 1);

   assert(strlen(prev_key) == KEY_SIZE);

   for (i = KEY_SIZE - 1; i >= 0; i--) {
      next_key[i] = add_char(prev_key[i], carry, &carry);

      if (i == 0) {
         assert(carry == 0);
      }
   }
   return next_key;
}

char last_key[] = "111111";
HTHandle h_table = NULL;

#define TINYURL "http://tinyurl.com/"
#define TINYURL_LEN 19

/** Encodes a URL to a shortened URL. */
char* encode(char* longUrl) {
   char *key;
   char *tinyurl = malloc(TINYURL_LEN + KEY_SIZE + 1);

   if (!h_table) {
      h_table = ht_alloc(1000, sdbm_str, key_alloc_fn_str,
                         compare_key_str, compare_data_str,
                         print_kv_str);
   }

   if (!tinyurl) {
      return NULL;
   }
   strcpy(tinyurl, TINYURL);

   key = next_key(last_key);
   if (!key) {
      return NULL;
   }

   if (ht_insert(h_table, key, longUrl) != OK) {
      return NULL;
   }

   return strcat(tinyurl, key);
}

/** Decodes a shortened URL to its original URL. */
char* decode(char* shortUrl) {
   char *key, *tmp;

   if (!h_table) {
      return NULL;
   }

   tmp = strstr(shortUrl, TINYURL);
   if (!tmp) {
      return NULL;
   }

   key = tmp + TINYURL_LEN;
   return ht_search(h_table, key);
}


// below for testing
int main()
{
   char *url = encode("https://leetcode.com/problems/design-tinyurl");

   printf("shortUrl - %s\n", url);
   url = decode(url);
   printf("LongUrl - %s\n", url);
}
