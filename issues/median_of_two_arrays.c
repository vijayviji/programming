#include <stdio.h>
#include <malloc.h>
#include <math.h>


#define MIN(a, b) (((a) < (b))? (a): (b))
#define MAX(a, b) (((a) > (b))? (a): (b))

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
   int start, end, i, median_num, j, *tmp, tmpSize;
   double median, max_of_left, min_of_right;

   // swap nums1 and nums2
   if (nums1Size > nums2Size) {
      tmpSize = nums1Size;
      nums1Size = nums2Size;
      nums2Size = tmpSize;

      tmp = nums1;
      nums1 = nums2;
      nums2 = tmp;
   }

   start = 0;
   end = nums1Size;

   /**
    * i, j tracks the space between numbers.
    * Let's say array A = [1, 2, 3, 4]
    * if i == 0, then the split is [], [1, 2, 3, 4]
    * if i == 4, then the split is [1, 2, 3, 4] []
    * if i == 1, then the split is [1, 2], [3, 4]
    * you can imagine the array as [_, 1, _, 2, _, 3, _, 4, _], i denotes dashes.
    *
    * this is similar for j as well
    */

   median_num = (nums1Size + nums2Size + 1) / 2;

   while (start <= end) {
      i = (start + end) / 2;
      j = median_num - i;

      if (i > 0 && nums1[i - 1] > nums2[j]) { // also, if i > 0, then j < nums2Size
         end = i - 1;
      } else if ((j > 0 && nums2[j - 1] > nums1[i])) { // also, if j > 0, then i < nums1Size
         start = i + 1;
      } else {
         break;
      }
   }

   if (i == 0) {
      max_of_left = nums2[j - 1];
   } else if (j == 0) {
      max_of_left = nums1[i - 1];
   } else {
      max_of_left = MAX(nums1[i - 1], nums2[j - 1]);
   }

   if (i == nums1Size) {
      min_of_right = nums2[j];
   } else if (j == nums2Size) {
      min_of_right = nums1[i];
   } else {
      min_of_right = MIN(nums1[i], nums2[j]);
   }

   if ((nums1Size + nums2Size) % 2 == 0) {
      return (max_of_left + min_of_right) / 2;
   } else {
      return max_of_left;
   }
}


int main()
{
   int nums1Size, nums2Size, *nums1, *nums2, i;

   printf("Enter nums1Size: ");
   scanf("%d", &nums1Size);
   printf("Enter nums2Size: ");
   scanf("%d", &nums2Size);

   nums1 = (int *) malloc(sizeof(int) * nums1Size);
   nums2 = (int *) malloc(sizeof(int) * nums2Size);

   printf("Enter nums1: ");
   for (i = 0; i < nums1Size; i++) {
      scanf("%d", &nums1[i]);
   }

   printf("Enter nums2: ");
   for (i = 0; i < nums2Size; i++) {
      scanf("%d", &nums2[i]);
   }

   printf("The median is %lf\n", findMedianSortedArrays(nums1, nums1Size,
            nums2, nums2Size));

   return 0;
}
