#include <stdio.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

typedef struct ListNode ListNode;

/**
 * Running Time analysis:
 *    O(kn)  k - no. of lists, n - length of the longest list.
 */
struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    ListNode *result = NULL, *min = NULL, *result_tail = NULL, **min_addr;
    uint32_t i;

    while (1) {
        min = NULL;
        for (i = 0; i < listsSize; i++) {
            if (!lists[i]) {
                continue;
            }

            if (min == NULL || lists[i]->val < min->val) {
                min = lists[i];
                min_addr = &lists[i];
            }
        }

        if (!min) {
            break;
        } else {
            (*min_addr) = (*min_addr)->next;
        }

        if (result == NULL) {
            result = min;
            result_tail = min;
        } else {
            result_tail->next = min;
            result_tail = result_tail->next;
        }
    }

    return result;
}
