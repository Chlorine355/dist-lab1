#include <stdio.h>
#include <stdlib.h>
#include "generateData.c"
#include <mpi.h>


void multiplyByRows(const float *matrix, const float *vector, float *result, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i] += matrix[i * cols + j] * vector[j];
        }
    }
}

void clearVector(float *vector, int size) {
    for (int i = 0; i < size; ++i) {
        vector[i] = 0.0f;
    }
}

void loadSerialTimes(double *times, int size) {
    FILE *file = fopen("times_by_rows.txt", "r");
    if (file == NULL) {
        perror("Couldn't open file");
        return;
    }
    for (int i = 0; i < size; ++i) {
        fscanf(file, "%lf", &times[i]);
    }
    fclose(file);
}

void saveSerialTimes(double *times, int size) {
    FILE *file = fopen("times_by_rows.txt", "w");
    if (file == NULL) {
        perror("Couldn't open file");
        return;
    }
    for (int i = 0; i < size; ++i) {
        fprintf(file, "%lf\n", times[i]);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sizes[][2] = {{1000,  1000},
                      {5000,  5000},
                      {10000, 10000}};
    int nSizes = 3;

    double T_serial[3] = {0.0, 0.0, 0.0};

    if (rank == 0) {
        if (size == 1) {
            printf("Running sequential test...\n");
        } else {
            printf("Running parallel test with %d processes...\n", size);
            loadSerialTimes(T_serial, nSizes);
        }
        printf("Processes\tMatrix Size\tTime (s)\tBoost\t\tEfficiency\n");
    }

    for (int test = 0; test < nSizes; ++test) {
        int rows = sizes[test][0];
        int cols = sizes[test][1];

        float startTime, endTime, duration = 0;

        int localRows = rows / size;
        int localElems = localRows * cols;

        float *matrix = NULL;
        float *vector = (float *) malloc(cols * sizeof(float));
        float *localMatrix = (float *) malloc(localElems * sizeof(float));
        float *localVec = (float *) malloc(localRows * sizeof(float));
        float *globalResult = NULL;

        if (rank == 0) {
            matrix = (float *) malloc(rows * cols * sizeof(float));
            globalResult = (float *) malloc(rows * sizeof(float));
            generateData(matrix, vector, globalResult, rows, cols);
        }

        MPI_Scatter(matrix, localElems, MPI_FLOAT, localMatrix, localElems, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(vector, cols, MPI_FLOAT, 0, MPI_COMM_WORLD);

        size_t iters = 10;
        for (size_t i = 0; i < iters; ++i) {
            startTime = MPI_Wtime();
            multiplyByRows(localMatrix, vector, localVec, localRows, cols);
            endTime = MPI_Wtime();
            duration += endTime - startTime;
            clearVector(localVec, localRows);
        }

        duration /= iters;

        multiplyByRows(localMatrix, vector, localVec, localRows, cols);

        MPI_Gather(localVec, localRows, MPI_FLOAT, globalResult, localRows, MPI_FLOAT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            size_t rowsRemaining = rows % size;
            size_t offset = rows - rowsRemaining;
            startTime = MPI_Wtime();
            multiplyByRows(matrix + offset * cols, vector, globalResult + offset, rowsRemaining, cols);
            endTime = MPI_Wtime();
            duration += endTime - startTime;
        }

        if (rank == 0) {
            if (size == 1) {
                T_serial[test] = duration;
            }

            double boost = (T_serial[test] > 0) ? T_serial[test] / duration : 1.0;
            double efficiency = (T_serial[test] > 0) ? boost / size : 0.0;

            printf("%d\t\t%dx%d\t\t%.6f\t%.2f\t\t%.2f\n", size, rows, cols, duration, boost, efficiency);
        }

        free(vector);
        free(localMatrix);
        free(localVec);
        if (rank == 0) {
            free(matrix);
            free(globalResult);
        }
    }

    if (rank == 0 && size == 1) {
        saveSerialTimes(T_serial, nSizes);
    }

    MPI_Finalize();
    return 0;
}
