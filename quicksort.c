#include <malloc.h>
#include <stdio.h>


int qs_select(int *nums, int p, int q)
{
    int i, j = p - 1, target = nums[q], tmp;
    
    for (i = p; i <= q; i++) {
        if (nums[i] < target) {
            j++;
            
            tmp = nums[j];
            nums[j] = nums[i];
            nums[i] = tmp;
        }
    }
    
    j++;
    tmp = nums[j];
    nums[j] = nums[q];
    nums[q] = tmp;
    
    return j;
}

void quicksort(int *nums, int p, int q)
{
    int r;
    
    if (p > q) {
        return;
    }
    
    r = qs_select(nums, p, q);
    
    quicksort(nums, p, r - 1);
    quicksort(nums, r + 1, q);
}

int main()
{
#define MAX 10
   int *nums = malloc(sizeof(int) * MAX);
   int tmp[MAX] = {8, 9, 4, 5, 3, 4, 2, 1, 0, 10}, i;

   for (i = 0; i < MAX; i++) {
      nums[i] = tmp[i];
   }

   quicksort(nums, 0, MAX - 1);

   for (i = 0; i < MAX; i++) {
      printf("%d ", nums[i]);  
   }

   printf("\n");

   return 0;
}
