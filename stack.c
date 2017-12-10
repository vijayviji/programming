#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <assert.h>

typedef enum ReturnStatus {
   ok = 0,
   fail = 1,
   no_mem = 2
} ReturnStatus;

typedef struct Node {
   void *data;
   struct Node *next;
} Node;

typedef struct Stack {
   int size;
   Node *top;
} Stack;

typedef Stack * StackHandle;

void push(StackHandle s, Node *n) {
   n->next = s->top;
   s->top = n;
   s->size++;
}

Node * pop(StackHandle s) {

   Node *tmp = NULL;

   if (!s->size) {
      return NULL;
   }

   tmp = s->top;
   s->top = s->top->next;
   s->size--;

   return tmp;
}

void init_stack(StackHandle s)
{
   s->size = 0;
   s->top = NULL;
}

Node * alloc_node(void *data) {
   Node *tmp = malloc(sizeof(Node));

   if (tmp != NULL) {
      tmp->data = data;
      tmp->next = NULL;
   }

   return tmp;
}

ReturnStatus push_data(StackHandle st, void *data) {
   Node *tmp = alloc_node(data);

   if (!tmp) {
      return no_mem;
   }

   push(st, data);

   return ok;
}
