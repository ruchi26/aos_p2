#include <omp.h>
#include <stdio.h>
#include <unistd.h>
#include "gtmp.h"
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

int barrier_count;
int epochs;
int sense;
int* local_sense;
// double* time_measurement;
int num_threads_global;

void gtmp_init(int num_threads) {
    if (num_threads < 1) {
        printf("Invalid number of threads\n");
    }
    num_threads_global = num_threads;
    printf("num_threads is %d\n", num_threads);
    barrier_count = num_threads;
    sense = 0;
    local_sense = calloc(num_threads, sizeof(int));
    for (int i = 0; i < num_threads; i++) {
        local_sense[i] = 0;
    }
    // time_measurement = calloc(num_threads * 100, sizeof(double));
}

void gtmp_barrier() {
    // struct timeval time_curr_finish;
    local_sense[omp_get_thread_num()] = !local_sense[omp_get_thread_num()];
    int local_check = 0;
#pragma omp critical 
    {
        barrier_count = barrier_count - 1;
        local_check = barrier_count;
    }

    if (local_check == 0) {
#pragma omp critical
        epochs = epochs + 1;
        {
            barrier_count = omp_get_num_threads();
            sense = !sense;
        }
    }
    else {
        while (sense != local_sense[omp_get_thread_num()]) {
            // Don't do anything...
            // printf("epoch %d waiting thread %d\n", epochs, omp_get_thread_num());
        }
    }
    // gettimeofday(&time_curr_finish, NULL);
    // time_measurement = ((double)(time_curr_end.tv_sec * 1000000) + time_curr_end.tv_usec) - ((double)(time_curr_start.tv_sec * 1000000) + time_curr_start.tv_usec);
    // time_measurement[(omp_get_num_threads() * (epochs - 1)) + omp_get_thread_num()] = (double)(time_curr_finish.tv_sec * 1000000) + time_curr_finish.tv_usec;
}

void gtmp_finalize() {
    free(local_sense);
    // double final_val = 0;
    // int adds = 0;
    // double min = __DBL_MAX__;
    // for (int i = 0; i < 100; i++) {
    //     for (int j = 0; j < omp_get_num_threads(); j++) {
    //         if (min > time_measurement[(omp_get_num_threads() * i) + j]) {
    //             min = time_measurement[(omp_get_num_threads() * i) + j];
    //         }
    //     }
    //     if (min > 100.0) {
    //         continue;
    //     }
    //     adds = adds + 1;
    //     final_val = final_val + min;
    // }
    // printf("min avg is %.9f with adds %d\n", final_val/adds, adds);
    // double curr_val = 0.0;
    // double variance = 0.0;
    // double total_variance = 0.0;
    // for (int i = 0; i < 100; i++) {
    //     for (int j = 0; j < num_threads_global; j++) {
    //         // printf("thread %d %.9f\n", j, time_measurement[(num_threads_global * i) + j]);
    //         curr_val = curr_val + time_measurement[(num_threads_global * i) + j];
    //     }
    //     curr_val = curr_val/num_threads_global;
    //     for (int j = 0; j < num_threads_global; j++) {
    //         variance = fabs((time_measurement[(num_threads_global * i) + j] - curr_val));
    //     }
    //     variance = variance/num_threads_global;
    //     total_variance = total_variance + variance;
    //     curr_val = 0.0;
    //     variance = 0.0;
    // }
    // printf("variance is %.9f\n", total_variance/100);
    // free(time_measurement);
}

