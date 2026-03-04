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
    int num_threads, num_iter=1;

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


    #pragma omp parallel
    {
        int i;
        for(i = 0; i < num_iter; i++){
        
        struct timeval tv;
        gettimeofday(&tv, NULL);
        double t_enter = tv.tv_sec + tv.tv_usec / 1000000.0;
        fprintf(stderr, "Thread [%d] before barrier at %.9f\n", omp_get_thread_num(), t_enter);
        gtmp_barrier();
        gettimeofday(&tv, NULL);
        double t_leave = tv.tv_sec + tv.tv_usec / 1000000.0;
        fprintf(stderr, "Thread [%d] after barrier at %.9f\n", omp_get_thread_num(), t_leave);
        }
    }

    printf("process %d entering barrier \n", my_rank);
    fflush(stdout);
    gtmpi_barrier();
    printf("process %d left barrier \n", my_rank);
    fflush(stdout);
    
    gtmp_finalize();

    gtmpi_finalize();  

    MPI_Finalize();

    return 0;
}
