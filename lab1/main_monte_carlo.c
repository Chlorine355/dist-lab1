#include <stdio.h>
#include <stdlib.h>
#include <monte_carlo.h>
#include "timer.h"

void time_monte_carlo(FILE *file, long nthreads, long ntrials) {
    double start, end;
    GET_TIME(start);
    char threads_str[10], trials_str[20];
    sprintf(threads_str, "%ld", nthreads);
    sprintf(trials_str, "%ld", ntrials);
    char *args[] = {"program", threads_str, trials_str};
    monte_carlo(3, args);
    GET_TIME(end);
    double time_taken = end - start;
    printf("Threads: %ld, Trials: %ld, Time: %f seconds\n", nthreads, ntrials, time_taken);
    fprintf(file, "%ld,%ld,%f\n", nthreads, ntrials, time_taken);
}

int main() {
    long trials[] = {10000, 100000, 1000000, 10000000, 100000000};
    long thread_counts[] = {1, 4, 8, 16, 32, 64, 100};
    FILE *file = fopen("monte_carlo_timed.csv", "w");
    if (!file) {
        perror("can't open file");
        return -1;
    }
    fprintf(file, "Threads,Trials,Time\n");
    for (int i = 0; i < sizeof(trials) / sizeof(trials[0]); ++i) {
        for (int j = 0; j < sizeof(thread_counts) / sizeof(thread_counts[0]); ++j) {
            time_monte_carlo(file, thread_counts[j], trials[i]);
        }
    }
    fclose(file);
    return 0;
}