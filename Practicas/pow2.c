#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void swap(long double *a, long double *b)  
{  
    long double aux = *a;  
    *a = *b;  
    *b = aux;  
} 

int partition (long double arr[], int low, int high) 
{ 
    long double pivot = arr[high];    // pivot 
    int i = (low - 1);  // Index of smaller element 
  
    for (int j = low; j <= high- 1; j++) 
    { 
        // If current element is smaller than the pivot 
        if (arr[j] < pivot) 
        { 
            i++;    // increment index of smaller element 
            swap(&arr[i], &arr[j]); 
        } 
    } 
    swap(&arr[i + 1], &arr[high]); 
    return (i + 1); 
} 
  
/* The main function that implements QuickSort 
 arr[] --> Array to be sorted, 
  low  --> Starting index, 
  high  --> Ending index */
void quickSort(long double arr[], int low, int high) 
{ 
    if (low < high) 
    { 
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int pi = partition(arr, low, high); 
  
        // Separately sort elements before 
        // partition and after partition 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
} 

void compareArrays(long double *arr, long double *arr2, int len)
{
    for(int i = 0; i < len; i++)
        printf("%Lf\t%Lf\n", arr[i], arr2[i]);
}

int main()
{
    int arrLen = 0;
    printf("Introduce arraLen: \n");
    scanf("%d", &arrLen);

    long double *arr = (long double *)malloc(sizeof(long double) * arrLen);
    long double *arr2 = (long double *)malloc(sizeof(long double) * arrLen);

    for(int i = 0; i < arrLen; i++)
    {
        arr2[i] = arr[i] = rand()%100;
    }
        
    quickSort(arr2, 0, arrLen-1);

    compareArrays(arr, arr2, arrLen);

    

    return 0;

}
    