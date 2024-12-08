#include <stdio.h>
#include <stdlib.h>
#include "generateData.c"
#include <mpi.h>


void multiplyByCols(const float *matrix, const float *vector, float *result, int rows, int cols, int startCol,
                    int endCol) {
    for (int i = 0; i < rows; ++i) {
        for (int j = startCol; j < endCol; ++j) {
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
    FILE *file = fopen("times_by_cols.txt", "r");
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
    FILE *file = fopen("times_by_cols.txt", "w");
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

        double startTime, endTime, duration = 0;
        
        float *matrix = (float *) malloc(rows * cols * sizeof(float));
        float *vector = (float *) malloc(cols * sizeof(float));
        float *localResult = (float *) malloc(rows * sizeof(float));
        float *globalResult = NULL;

        for (int i = 0; i < rows; ++i) {
            localResult[i] = 0.0f;
        }

        if (rank == 0) {
            globalResult = (float *) malloc(rows * sizeof(float));
            generateData(matrix, vector, globalResult, rows, cols);
        }

        MPI_Bcast(vector, cols, MPI_FLOAT, 0, MPI_COMM_WORLD);

        MPI_Bcast(matrix, rows * cols, MPI_FLOAT, 0, MPI_COMM_WORLD);

        int currentCols = 1, startCol = 0, endCol = 0;
        if (rank < cols) {
            if (size <= cols) {
                currentCols = cols / size;
            }
            startCol = rank * currentCols;
            endCol = startCol + currentCols;
            if (rank == size - 1) {
                endCol = cols;
            }
        }

        int iters = 10;
        for (size_t i = 0; i < iters; ++i) {
            if (rank < cols) {
                startTime = MPI_Wtime();
                multiplyByCols(matrix, vector, localResult, rows, cols, startCol, endCol);
                endTime = MPI_Wtime();
                duration += endTime - startTime;
                clearVector(localResult, rows);
            }
        }

        duration /= (double) iters;

        if (rank < cols) {
            multiplyByCols(matrix, vector, localResult, rows, cols, startCol, endCol);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Reduce(localResult, globalResult, rows, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            if (size == 1) {
                T_serial[test] = duration;
            }

            double boost = (T_serial[test] > 0) ? T_serial[test] / duration : 1.0;
            double efficiency = (T_serial[test] > 0) ? boost / size : 0.0;

            printf("%d\t\t%dx%d\t\t%.6f\t%.2f\t\t%.2f\n", size, rows, cols, duration, boost, efficiency);
        }

        free(localResult);
        free(vector);
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
