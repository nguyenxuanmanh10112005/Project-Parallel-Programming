#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define N 10000000 

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int left, int right) {
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

void quicksort_parallel(int arr[], int left, int right, int depth) {
    if (left < right) {
        int pi = partition(arr, left, right);

        if (depth < 4) {
            #pragma omp task shared(arr)
            quicksort_parallel(arr, left, pi - 1, depth + 1);

            #pragma omp task shared(arr)
            quicksort_parallel(arr, pi + 1, right, depth + 1);

            #pragma omp taskwait
        } else {
            quicksort_parallel(arr, left, pi - 1, depth + 1);
            quicksort_parallel(arr, pi + 1, right, depth + 1);
        }
    }
}

int main() {
    int num_threads;

    printf("Nhap so luong: ");
    scanf("%d", &num_threads);

    int* arr = (int*)malloc(N * sizeof(int));
    if (!arr) {
        printf("Khong the cap phat bo nho!\n");
        return 1;
    }

    srand((unsigned int)time(NULL));
    for (int i = 0; i < N; i++) {
        arr[i] = rand();
    }

    omp_set_num_threads(num_threads);

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        quicksort_parallel(arr, 0, N - 1, 0);
    }

    double end = omp_get_wtime();

    printf("Da sap xep %d phan tu bang %d luong trong %.6f giay \n", N, num_threads, end - start);

    free(arr);
    return 0;
}
