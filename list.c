#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>

typedef enum ReturnStatus {
   OK = 0,
   FAIL = 1,
   NO_MEM = 2
} ReturnStatus;

typedef struct ListNode {
   void *data;
   struct ListNode *prev;
   struct ListNode *next;
} ListNode;

#define LIST_FOR_ALL(list, node_ptr) \
   for (node_ptr = LIST_FIRST(list); \
        !LIST_IS_AT_END(list, node_ptr); \
        node_ptr = LIST_NEXT(node_ptr))

// to be used when removing items in the list
#define LIST_FOR_ALL_SAFE(list, node_ptr, next_ptr) \
   for (node_ptr = LIST_FIRST(list), next_ptr = LIST_NEXT(LIST_FIRST(list)); \
        !LIST_IS_AT_END(list, node_ptr); \
        node_ptr = next_ptr, next_ptr = LIST_NEXT(next_ptr))

#define LIST_FIRST(list) list->head->next
#define LIST_LAST(list) list->head->prev
#define LIST_NEXT(node_ptr) node_ptr->next

#define LIST_IS_AT_END(list, node_ptr) (node_ptr == list->head)
#define LIST_IS_EMPTY(list) (list->head->next == list->head)

typedef void (*ListPrintData)(void *);

typedef struct List {
   ListNode *head;
   ListPrintData print_data_fn;
} List;

void list_insert_at_front(List *list, ListNode *node)
{
   ListNode *head = list->head;


   node->next = head->next;
   head->next->prev = node;

   node->prev = head;
   head->next = node;
}

void list_insert_at_rear(List *list, ListNode *node)
{
   ListNode *head = list->head;


   node->prev = head->prev;
   head->prev->next = node;

   node->next = head;
   head->prev = node;
}

ListNode * list_alloc_element(void *data)
{
   ListNode *tmp = malloc(sizeof(ListNode));

   if (!tmp) {
      return NULL;
   }

   tmp->next = NULL;
   tmp->prev = NULL;
   tmp->data = data;

   return tmp;
}

ReturnStatus list_insert_data(List *list, void *data, bool at_front)
{
   ListNode *tmp = list_alloc_element(data);

   if (!tmp) {
      return NO_MEM;
   }

   if (at_front) {
      list_insert_at_front(list, tmp);
   } else {
      list_insert_at_rear(list, tmp);
   }

   return OK;
}

void list_item_unlink(ListNode *node)
{
   node->prev->next = node->next;
   node->next->prev = node->prev;
}

void list_item_free(ListNode *node, bool destroy_data)
{
   list_item_unlink(node);
   if (destroy_data) {
      free(node);
   }
}

List * list_alloc(ListPrintData print_data_fn)
{
   List *list = malloc(sizeof(List));
   ListNode *head;

   if (!list) {
      return NULL;
   }

   head = list_alloc_element((void *) NULL);
   if (!head) {
      free(list);
      return NULL;
   }

   head->next = head;
   head->prev = head;

   list->head = head;
   list->print_data_fn = print_data_fn;

   return list;
}

void list_destroy(List *list, bool destroy_data)
{
   ListNode *itr, *next;

   if (!list) {
      return;
   }

   LIST_FOR_ALL_SAFE(list, itr, next) {
      list_item_free(itr, destroy_data);
   }

   free(list);
}

void list_print(List *list)
{
   ListNode *itr;

   if (!list) {
      return;
   }

   LIST_FOR_ALL(list, itr) {
      list->print_data_fn(itr->data);
      printf(" -> ");
   }

   printf("\n");
}

/* following for testing */

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
   list_destroy(list);
   list = NULL;
}
