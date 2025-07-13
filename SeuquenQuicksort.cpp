#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Hoán đổi hai giá trị
void swap(int* a, int* b) {
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

// Quick Sort tăng dần
void quicksort_asc(int arr[], int left, int right) {
    if (left < right) {
        int pi = partition_asc(arr, left, right);
        quicksort_asc(arr, left, pi - 1);
        quicksort_asc(arr, pi + 1, right);
    }
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

// Quick Sort giảm dần
void quicksort_desc(int arr[], int left, int right) {
    if (left < right) {
        int pi = partition_desc(arr, left, right);
        quicksort_desc(arr, left, pi - 1);
        quicksort_desc(arr, pi + 1, right);
    }
}

int main() {
    int N;
    printf("Nhap so luong phan tu N: ");
    scanf("%d", &N);

    // Cấp phát động
    int* arr = (int*)malloc(N * sizeof(int));
    int* arr1 = (int*)malloc(N * sizeof(int));  // để sắp xếp tăng
    int* arr2 = (int*)malloc(N * sizeof(int));  // để sắp xếp giảm

    if (!arr || !arr1 || !arr2) {
        printf("Khong the cap phat bo nho!\n");
        return 1;
    }

    // Sinh dữ liệu ngẫu nhiên
    srand((unsigned int)time(NULL));
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % 1000000;
        arr1[i] = arr[i];
        arr2[i] = arr[i];
    }

    // Quick Sort tăng dần
    clock_t start_asc = clock();
    quicksort_asc(arr1, 0, N - 1);
    clock_t end_asc = clock();
    printf("Thoi gian sap xep tang dan: %.6f giay\n", (double)(end_asc - start_asc) / CLOCKS_PER_SEC);

    // Quick Sort giảm dần
    clock_t start_desc = clock();
    quicksort_desc(arr2, 0, N - 1);
    clock_t end_desc = clock();
    printf("Thoi gian sap xep giam dan: %.6f giay\n", (double)(end_desc - start_desc) / CLOCKS_PER_SEC);

    // Giải phóng bộ nhớ
    free(arr);
    free(arr1);
    free(arr2);

    return 0;
}
