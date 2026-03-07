#include <mpi.h>
#include <omp.h>
#include "gtmp.h"
#include "gtmpi.h" 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

int main(int argc, char** argv)
{
    int totalProcessCount, my_rank;
    int num_threads, num_iter=100;

    MPI_Init(&argc, &argv);
    
    if (argc < 2){
        fprintf(stderr, "Usage: ./combined [NUM_THREADS]\n");
        exit(EXIT_FAILURE);
    }

    num_threads = strtol(argv[1], NULL, 10);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcessCount);
    gtmpi_init(totalProcessCount);

    omp_set_dynamic(0);
    if (omp_get_dynamic())
        printf("Warning: dynamic adjustment of threads has been set\n");

    omp_set_num_threads(num_threads);
    
    gtmp_init(num_threads);

    // struct timeval time_start;
    // struct timeval time_end;
    double total_time = 0.0;
    double max_total_time = 0.0;
    for (int j = 0; j < num_iter; j++) {
        // gettimeofday(&time_start, NULL);
        double t0 = MPI_Wtime();
        #pragma omp parallel
        {
            int i;
            for (i = 0; i < num_iter; i++) {
                gtmp_barrier();
            }
        }
        gtmpi_barrier();
        double t1 = MPI_Wtime();
        // gettimeofday(&time_end, NULL);
        // total_time = total_time + ((double)(time_end.tv_sec * 1000000) + time_end.tv_usec) - ((double)(time_start.tv_sec * 1000000) + time_start.tv_usec);
        total_time += t1 - t0;
        if (total_time > max_total_time) {
            max_total_time = total_time;
        }
    }
    double g_lat_sum = 0.0, g_lat_max = 0.0;
    MPI_Reduce(&total_time, &g_lat_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&max_total_time, &g_lat_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    if (my_rank == 0) {
        
        // Average per barrier over all ranks
        double avg_latency = g_lat_sum / totalProcessCount / num_iter;

        printf("[AVG]Number of threads %d, number of processes %d, avg time %.9f\n", num_threads, totalProcessCount, avg_latency);
        printf("[MAX]Number of threads %d, number of processes %d, max time %.9f\n", num_threads, totalProcessCount, g_lat_max);
        fflush(stdout);
    }
    
    gtmp_finalize();

    gtmpi_finalize();  

    MPI_Finalize();

    return 0;
}
