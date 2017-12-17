#ifndef _LIST_H
#define _LIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>

#include "common.h"

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

#define LIST_FIRST(list) (list)->head.next
#define LIST_LAST(list) (list)->head.prev
#define LIST_NEXT(node_ptr) (node_ptr)->next

#define LIST_IS_AT_END(list, node_ptr) ((node_ptr) == &((list)->head))
#define LIST_IS_EMPTY(list) ((list)->head.next == &(list->head))

typedef void (*ListPrintData)(void *);

typedef struct List {
   ListNode head;
   ListPrintData print_data_fn;
} List;

typedef List * ListHandle;


void list_insert_at_front(List *list, ListNode *node);
void list_insert_at_rear(List *list, ListNode *node);
void list_init_element(ListNode *node, void *data);
void list_make_element_self_ref(ListNode *node);
ListNode * list_alloc_element(void *data);
ReturnStatus list_insert_data(List *list, void *data, bool at_front);
void list_item_unlink(ListNode *node);
void list_items_link(ListNode *node1, ListNode *node2);
void list_item_free(ListNode *node, bool destroy_data);
void list_init(List *list, ListPrintData print_data_fn);
List * list_alloc(ListPrintData print_data_fn);
void list_destroy(List *list, bool destroy_data);
void list_print(List *list);

#endif // _LIST_H
