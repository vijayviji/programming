#include <stdio.h>
#include <malloc.h>

typedef struct ListNode {
   int val;
   struct ListNode *next;
}ListNode;

struct ListNode* addTwoNumbers(struct ListNode* l1, struct ListNode* l2) {
    int carry = 0;
    struct ListNode *result, *tmp, *last;

    if (!l1 && !l2) {
        return NULL;
    }

    result = malloc(sizeof(struct ListNode));
    result->next = NULL;
    tmp = result;

    while(1) {

        tmp->val = carry;

        if (l1) {
            tmp->val += l1->val;
            l1 = l1->next;
        }

        if (l2) {
            tmp->val += l2->val;
            l2 = l2->next;
        }

        if (tmp->val >= 10) {
            carry = tmp->val / 10;
            tmp->val %= 10;
        } else {
            carry = 0;
        }

        if (!l1 && !l2 && !carry) {
            break;
        }

        tmp->next = malloc(sizeof(struct ListNode));
        last = tmp;
        tmp = tmp->next;
        tmp->next = NULL;
    }

    //free(last);

    return result;
}


/**
 * below test code
 */


ListNode *create_node(int i)
{
   ListNode *res = (ListNode *) malloc(sizeof(ListNode));

   res->val = i;
   res->next = NULL;

   return res;
}

void print_list(ListNode *l1)
{
   while(l1 != NULL) {
      printf("%d -> ", l1->val);
      l1 = l1->next;
   }

   printf("NULL\n");
}

int main()
{
   ListNode *n1 = create_node(0), *n2 = create_node(1), **n1Next, **n2Next;
   int i;

   n1Next = &(n1->next);
   n2Next = &(n2->next);

   for (i = 1; i <= 10; i++) {
      *n1Next = create_node(i);
      *n2Next = create_node(i);

      n1Next = &((*n1Next)->next);
      n2Next = &((*n2Next)->next);
   }

   print_list(n1);
   print_list(n2);
   print_list(addTwoNumbers(n1, n2));

}
