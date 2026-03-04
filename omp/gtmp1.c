#include <omp.h>
#include <stdio.h>
#include <unistd.h>
#include "gtmp.h"
#include <stdbool.h>
#include <stdlib.h>

int barrier_count;
bool sense;
bool* local_sense;

void gtmp_init(int num_threads) {
    if (num_threads < 1) {
        printf("Invalid number of threads\n");
    }
    printf("num_threads is %d\n", num_threads);
    barrier_count = num_threads;
    sense = true;
    local_sense = malloc(num_threads * sizeof(bool));
    for (int i = 0; i < num_threads; i++) {
        local_sense[i] = true;
    }
}

void gtmp_barrier() {
    local_sense[omp_get_thread_num()] = !local_sense[omp_get_thread_num()];
    #pragma omp critical 
    {
        // printf("thread %d reached barrier, barrier count is %d with address being %p\n", omp_get_thread_num(), barrier_count, &barrier_count);
        // fflush(stdout);
        barrier_count = barrier_count - 1;
        // printf("thread %d decremented barrier count to %d\n", omp_get_thread_num(), barrier_count);
        // fflush(stdout);
    }

    if (barrier_count == 0) {
        #pragma omp critical
        {
            barrier_count = omp_get_num_threads();
            sense = !sense;
        }
    }
    else {
        while (sense != local_sense[omp_get_thread_num()]) {
            // Don't do anything...
            // printf("thread %d is waiting with barrier count %d\n", omp_get_thread_num(), barrier_count);
            // fflush(stdout);
        }
    }
}

void gtmp_finalize() {
    printf("thread %d finalized\n", omp_get_thread_num());
}

