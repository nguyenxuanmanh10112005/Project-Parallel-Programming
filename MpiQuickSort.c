#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 10000000

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void quicksort(int *arr, int left, int right) {
    if (left < right) {
        int pivot = arr[right];
        int i = left - 1;
        for (int j = left; j < right; j++) {
            if (arr[j] <= pivot) {
                i++;
                swap(&arr[i], &arr[j]);
            }
        }
        swap(&arr[i + 1], &arr[right]);
        int pi = i + 1;
        quicksort(arr, left, pi - 1);
        quicksort(arr, pi + 1, right);
    }
}

void merge(int *a, int *b, int n, int m, int *result) {
    int i = 0, j = 0, k = 0;
    while (i < n && j < m) {
        if (a[i] <= b[j]) {
            result[k++] = a[i++];
        }
        else {
            result[k++] = b[j++];
        }
    }
    while (i < n) {
        result[k++] = a[i++];
    }
    while (j < m) {
        result[k++] = b[j++];
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int *data = NULL;
    int *local_data = NULL;
    int *counts = NULL;
    int *displs = NULL;
    int local_n;

    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Tính toán chia mảng đều cho các tiến trình (dùng Scatterv)
    if (rank == 0) {
        data = (int *)malloc(SIZE * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < SIZE; i++) {
            data[i] = rand();
        }

        counts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        int rem = SIZE % size;
        int sum = 0;
        for (int i = 0; i < size; i++) {
            counts[i] = SIZE / size + (i < rem ? 1 : 0);
            displs[i] = sum;
            sum += counts[i];
        }

        start_time = MPI_Wtime();
    }

    if (rank != 0) {
        counts = (int *)malloc(size * sizeof(int));
    }
    MPI_Bcast(counts, size, MPI_INT, 0, MPI_COMM_WORLD);
    local_n = counts[rank];
    local_data = (int *)malloc(local_n * sizeof(int));

    if (rank != 0)
        displs = (int *)malloc(size * sizeof(int));
    MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);

    // Gửi dữ liệu tới các tiến trình
    MPI_Scatterv(data, counts, displs, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Quick sort cục bộ
    quicksort(local_data, 0, local_n - 1);

    // Ghép từng bước
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int recv_size;
                MPI_Recv(&recv_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int *recv_data = (int *)malloc(recv_size * sizeof(int));
                MPI_Recv(recv_data, recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int *merged = (int *)malloc((local_n + recv_size) * sizeof(int));
                merge(local_data, recv_data, local_n, recv_size, merged);

                free(local_data);
                free(recv_data);
                local_data = merged;
                local_n += recv_size;
            }
        } else {
            int target = rank - step;
            MPI_Send(&local_n, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
            MPI_Send(local_data, local_n, MPI_INT, target, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("MPI Quick Sort completed in %.3f seconds with %d processes.\n", end_time - start_time, size);
        free(data);
        free(counts);
        free(displs);
        free(local_data);
    } else {
        free(counts);
        free(displs);
        free(local_data);
    }

    MPI_Finalize();
    return 0;
}

// gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
// mpiexec -n 14 MpiQuickSort.exe

/*
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> cd "c:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code\" ; if ($?) { gcc tempCodeRunnerFile.c -o tempCodeRunnerFile } ; if ($?) { .\tempCodeRunnerFile }
tempCodeRunnerFile.c:1:5: error: expected '=', ',', ';', 'asm' or '__attribute__' before '-' token
    1 | gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
      |     ^
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64"PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> cd "c:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code\" ; if ($?) { gcc tempCodeRunnerFile.c -o tempCodeRunnerFile } ; if ($?) { .\tempCodeRunnerFile }
tempCodeRunnerFile.c:1:5: error: expected '=', ',', ';', 'asm' or '__attribute__' before '-' token
    1 | gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
      |     ^
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64"PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> cd "c:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code\" ; if ($?) { gcc tempCodeRunnerFile.c -o tempCodeRunnerFile } ; if ($?) { .\tempCodeRunnerFile }
tempCodeRunnerFile.c:1:5: error: expected '=', ',', ';', 'asm' or '__attribute__' before '-' token
    1 | gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
      |     ^
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
    1 | gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
      |     ^
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64"PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 2 MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 2 MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 2 MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 2 MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 2 MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 2 MpiQuickSort.exe
MPI Quick Sort completed in 0.352 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 2 MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.353 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64"MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64"MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
MPI Quick Sort completed in 1.662 seconds with 2 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 4 MpiQuickSort.exe
MPI Quick Sort completed in 0.667 seconds with 4 processes.
MPI Quick Sort completed in 0.667 seconds with 4 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.353 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 10 MpiQuickSort.exe
MPI Quick Sort completed in 0.494 seconds with 10 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 12 MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.353 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 10 MpiQuickSort.exe
MPI Quick Sort completed in 0.494 seconds with 10 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.353 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
 -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 6 MpiQuickSort.exe
MPI Quick Sort completed in 0.436 seconds with 6 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 8 MpiQuickSort.exe
MPI Quick Sort completed in 0.353 seconds with 8 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 10 MpiQuickSort.exe
MPI Quick Sort completed in 0.494 seconds with 10 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 12 MpiQuickSort.exe
MPI Quick Sort completed in 0.458 seconds with 12 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> gcc -I"D:\MSMPI\Include" MpiQuickSort.c -L "D:\MSMPI\Lib\x64" -lmsmpi -o MpiQuickSort.exe
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code> mpiexec -n 14 MpiQuickSort.exe
MPI Quick Sort completed in 0.428 seconds with 14 processes.
PS C:\Users\ADMIN\Documents\University\2024.3\Introduction to Parallel Computing\Đồ án\Code>*/
