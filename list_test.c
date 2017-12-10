#include "list.h"

void print_int(void *data) {
   int *tmp = (int *) data;

   if (tmp) {
      printf("%d", *tmp);
   } else {
      printf("NULL");
   }
}

int main()
{
#define MAX_ELEM 10

   int a[MAX_ELEM] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, i;
   List *list = list_alloc(print_int);

   if (!list) {
      printf("List alloc failed");
      return 1;
   }

   for (i = 0; i < MAX_ELEM; i++) {
      list_insert_data(list, &a[i], false);
   }

   list_print(list);
   list_destroy(list, false);
   list = NULL;
}
