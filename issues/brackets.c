#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <assert.h>

typedef struct Node {
   char data;
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

Node * alloc_node(char data) {
   Node *tmp = malloc(sizeof(Node));

   if (tmp != NULL) {
      tmp->data = data;
      tmp->next = NULL;
   }

   return tmp;
}

bool isValid(char* s) {
   StackHandle st = malloc(sizeof(Stack));
   Node *tmp;
   bool status = true;

   if (!s) {
      return true;
   }

   if (!st) {
      printf("Error: Stack allocation failed");
      return false;
   }

   init_stack(st);
   while (*s != '\0') {
      if (*s == '[' || *s == '{'  || *s == '(') {
         tmp = alloc_node(*s);
         if (!tmp) {
            printf("Error: Node allocation failed");
            status = false;
            goto end;
         }
         push(st, tmp);
      } else if (*s == ']' || *s == '}'  || *s == ')') {
         tmp = pop(st);
         if (!tmp) {
            status = false;
            goto end;
         }

         switch(*s) {
            case ']':
               if (tmp->data != '[') {
                  status = false;
               }
               break;
            case '}':
               if (tmp->data != '{') {
                  status = false;
               }
               break;
            case ')':
               if (tmp->data != '(') {
                  status = false;
               }
               break;
         }
         free(tmp);

         if(!status) {
            goto end;
         }
      } else {
         assert(0);
      }
      s++;
   }

   if (st->size > 0) {
      status = false;
   } else {
      status = true;
   }

end:
   free(st);
   return status;
}

int main()
{
   char s[] = "(){}[]";
   printf("%s\n", ((isValid(s))? "true": "false"));

   return 0;
}
