#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 10000000

typedef struct {
    int *arr;
    int left;
    int right;
} ThreadData;

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
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

void quicksort(int arr[], int left, int right) {
    if (left < right) {
        int pi = partition(arr, left, right);
        quicksort(arr, left, pi - 1);
        quicksort(arr, pi + 1, right);
    }
}

void *threaded_quicksort(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    quicksort(data->arr, data->left, data->right);
    pthread_exit(NULL);
}

void merge(int *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = malloc(n1 * sizeof(int));
    int *R = malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++) {
        L[i] = arr[left + i];
    }
    for (int i = 0; i < n2; i++) {
        R[i] = arr[mid + 1 + i];
    }

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j])
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1)
        arr[k++] = L[i++];
    while (j < n2)
        arr[k++] = R[j++];

    free(L);
    free(R);
}

int main() {
    int P;
    printf("Nhap so luong thread (2, 4, 6, 8, 10, 12): ");
    scanf("%d", &P);

    if (P < 1 || P > N)
    {
        printf("So thread khong hop le.\n");
        return 1;
    }

    int *arr = malloc(N * sizeof(int));
    if (!arr) {
        printf("Khong cap phat duoc bo nho!\n");
        return 1;
    }

    srand((unsigned int)time(NULL));
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % 1000000;
    }

    pthread_t threads[P];
    ThreadData td[P];
    int chunk = N / P;

    clock_t start_parallel = clock();

    // Tạo các luồng sắp xếp độc lập
    for (int i = 0; i < P; i++) {
        td[i].arr = arr;
        td[i].left = i * chunk;
        td[i].right = (i == P - 1) ? (N - 1) : ((i + 1) * chunk - 1);
        pthread_create(&threads[i], NULL, threaded_quicksort, &td[i]);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(threads[i], NULL);
    }

    // Gộp dần từng đoạn đã sắp xếp
    int current_chunk = chunk;
    while (current_chunk < N) {
        for (int i = 0; i < N; i += 2 * current_chunk) {
            int mid = i + current_chunk - 1;
            int right = ((i + 2 * current_chunk - 1) < N) ? (i + 2 * current_chunk - 1) : (N - 1);
            if (mid < right)
                merge(arr, i, mid, right);
        }
        current_chunk *= 2;
    }

    clock_t end_parallel = clock();
    double time_parallel = (double)(end_parallel - start_parallel) / CLOCKS_PER_SEC;

    printf("Thoi gian sap xep bang pthreads (N = %d, P = %d): %.6f giay\n", N, P, time_parallel);

    // Kiểm tra mảng đã sắp đúng
    for (int i = 1; i < N; i++) {
        if (arr[i - 1] > arr[i]) {
            printf("Loi: Mang chua duoc sap xep dung tai vi tri %d\n", i - 1);
            break;
        }
    }
    free(arr);
    return 0;
}
