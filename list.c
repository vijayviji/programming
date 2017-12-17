#include "list.h"

void list_insert_at_front(List *list, ListNode *node)
{
   ListNode *head = &list->head;

   node->next = head->next;
   head->next->prev = node;

   node->prev = head;
   head->next = node;
}

void list_insert_at_rear(List *list, ListNode *node)
{
   ListNode *head = &list->head;


   node->prev = head->prev;
   head->prev->next = node;

   node->next = head;
   head->prev = node;
}

void list_init_element(ListNode *node, void *data)
{
   node->next = NULL;
   node->prev = NULL;
   node->data = data;
}

void list_make_element_self_ref(ListNode *node)
{
   node->next = node;
   node->prev = node;
}

ListNode * list_alloc_element(void *data)
{
   ListNode *tmp = malloc(sizeof(ListNode));

   if (!tmp) {
      return NULL;
   }

   list_init_element(tmp, data);
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

void list_items_link(ListNode *node1, ListNode *node2)
{
   ListNode *node1Next = node1->next, *node2Prev = node2->prev;

   node1->next = node2;
   node1Next->prev = node2Prev;

   node2->prev = node1;
   node2Prev->next = node1Next;
}

void list_item_free(ListNode *node, bool destroy_data)
{
   list_item_unlink(node);
   if (destroy_data) {
      free(node);
   }
}

void list_init(List *list, ListPrintData print_data_fn)
{
   list->head.next = &list->head;
   list->head.prev = &list->head;
   list->print_data_fn = print_data_fn;
}

List * list_alloc(ListPrintData print_data_fn)
{
   List *list = malloc(sizeof(List));

   if (!list) {
      return NULL;
   }

   list_init(list, print_data_fn);
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
