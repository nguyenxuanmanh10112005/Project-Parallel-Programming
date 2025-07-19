#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000000  // 10 triệu

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Phân hoạch tăng dần
int partition_asc(int arr[], int left, int right) {
    int pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[right]);
    return i + 1;
}

// Phân hoạch giảm dần
int partition_desc(int arr[], int left, int right) {
    int pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (arr[j] > pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[right]);
    return i + 1;
}

// Quicksort tăng dần
void quicksort_asc(int arr[], int left, int right) {
    if (left < right) {
        int pivot = partition_asc(arr, left, right);
        quicksort_asc(arr, left, pivot - 1);
        quicksort_asc(arr, pivot + 1, right);
    }
}

// Quicksort giảm dần
void quicksort_desc(int arr[], int left, int right) {
    if (left < right) {
        int pivot = partition_desc(arr, left, right);
        quicksort_desc(arr, left, pivot - 1);
        quicksort_desc(arr, pivot + 1, right);
    }
}

void generate_random_array(int arr[], int n) {
    for (int i = 0; i < n; i++)
        arr[i] = rand();
}

void copy_array(int src[], int dest[], int n) {
    for (int i = 0; i < n; i++)
        dest[i] = src[i];
}

int main() {
    int *arr = (int *)malloc(N * sizeof(int));
    int *arr_copy = (int *)malloc(N * sizeof(int));
    srand(time(NULL));

    generate_random_array(arr, N);

    copy_array(arr, arr_copy, N);

    // sx tăng dần
    clock_t start_asc = clock();
    quicksort_asc(arr, 0, N - 1);
    clock_t end_asc = clock();
    double time_asc = (double)(end_asc - start_asc) / CLOCKS_PER_SEC;

    // sx giảm dần
    clock_t start_desc = clock();
    quicksort_desc(arr_copy, 0, N - 1);
    clock_t end_desc = clock();
    double time_desc = (double)(end_desc - start_desc) / CLOCKS_PER_SEC;

    double avg_time = (time_asc + time_desc) / 2.0;

    printf("Sequential QuickSort\n");
    printf("Thoi gian sap xep tang dan : %.4f seconds\n", time_asc);
    printf("Thoi gian sap xep giam dan: %.4f seconds\n", time_desc);
    printf("Thoi gian trung binh : %.4f seconds\n", avg_time);

    free(arr);
    free(arr_copy);
    return 0;
}
