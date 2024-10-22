#include <stdio.h>
#include <stdlib.h>
#include "mandelbrot.h"
#include "timer.h"

void time_mandelbrot(FILE *file, long nthreads, long npoints) {
    double start, end;
    GET_TIME(start);
    char threads_str[10], points_str[20];
    sprintf(threads_str, "%ld", nthreads);
    sprintf(points_str, "%ld", npoints);
    char *args[] = {"program", threads_str, points_str};
    mandelbrot(3, args);
    GET_TIME(end);
    double time_taken = end - start;
    printf("Threads: %ld, Points: %ld, Time: %f seconds\n", nthreads, npoints, time_taken);
    fprintf(file, "%ld,%ld,%f\n", nthreads, npoints, time_taken);
}

int main() {
    long points[] = {10, 100, 1000, 10000, 100000};
    long thread_counts[] = {1, 4, 8, 16, 32, 64, 100};
    FILE *file = fopen("mandelbrot_timed.csv", "w");
    if (!file) {
        printf("can't open file");
        return -1;
    }
    fprintf(file, "Threads,Points,Time\n");
    for (int i = 0; i < sizeof(points) / sizeof(points[0]); ++i) {
        for (int j = 0; j < sizeof(thread_counts) / sizeof(thread_counts[0]); ++j) {
            time_mandelbrot(file, thread_counts[j], points[i]);
        }
    }
    fclose(file);
    return 0;
}