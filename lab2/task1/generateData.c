#include <stdlib.h>

void generateData(float *matrix, float *vector, float *result, int rows, int cols) {
    for (int i = 0; i < rows * cols; ++i) {
        matrix[i] = (float) (rand() % 100) / 10.0f;
    }

    for (int i = 0; i < cols; ++i) {
        vector[i] = (float) (rand() % 100) / 10.0f;
    }

    for (int i = 0; i < rows; ++i) {
        result[i] = 0.0f;
    }
}