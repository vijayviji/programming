#include <stdio.h>
#include <malloc.h>
#include "../list.h"
#include "../hashtable.h"

typedef struct NodeData {
   int value; // for debugging purpose
   bool visited;
} NodeData;

// space: O(n)
// time: O(n)
int longestConsecutive(int* nums, int numsSize) {
   HTHandle htable = ht_alloc(numsSize, ht_hash_clrs_int_fn, key_alloc_fn_int_fn,
                              compare_key_int_fn, NULL, NULL);
   int i = 0, longest_consec_len = 0, tmp_int = 0;
   ListNode *tmpListNode = NULL, *nextnode = NULL, *curnode = NULL;
   NodeData *tmpNodeData = NULL;

   for (i = 0; i < numsSize; i++) {
      tmpNodeData = (NodeData *) malloc(sizeof(NodeData));
      assert(tmpNodeData != NULL);

      tmpNodeData->value = nums[i];
      tmpNodeData->visited = false;

      tmpListNode = list_alloc_element(tmpNodeData);
      assert(tmpListNode != NULL);
      list_make_element_self_ref(tmpListNode);

      ht_insert(htable, &nums[i], tmpListNode);
   }

   for (i = 0; i < numsSize; i++) {
      curnode = ht_search(htable, &nums[i]);

      tmp_int = nums[i] + 1;
      nextnode = ht_search(htable, &tmp_int);

      if (nextnode != NULL) {
         list_items_link(curnode, nextnode);
      }
   }

   for (i = 0; i < numsSize; i++) {
      tmp_int = 0;
      curnode = ht_search(htable, &nums[i]);
      tmpNodeData = (NodeData *) curnode->data;

      // Since we'll not proceed the chain of a node which is visited already,
      // the entire fn is O(n)
      while(!tmpNodeData->visited) {
         tmpNodeData->visited = true;
         tmp_int++;
         curnode = curnode->next;
         tmpNodeData = (NodeData *) curnode->data;
      }

      if (tmp_int > longest_consec_len) {
         longest_consec_len = tmp_int;
      }
   }

   return longest_consec_len;
}


int main()
{
#define ARRAY_SIZE 7
   int nums[] = {1, 3, 5, 7, 8, 10, 6};

   printf("Longest consecutive len: %d\n", longestConsecutive(nums, ARRAY_SIZE));
   return 0;
}
