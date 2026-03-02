#include <assert.h>
#include <mpi.h>
#include <omp.h>
#include "gtmp.h"
#include "gtmp_combined.h"
#include "gtmpi.h" 

static int provided_thread_support = MPI_THREAD_SINGLE;
static int hybrid_num_threads = 1;

// TODO: make a harness file to test and run this one

void gtmp_hybrid_init(int num_threads) {
    // using MPI with FUNNELED support so that there is only 1 thread that can make MPI calls.
    MPI_Query_thread(&provided_thread_support);
    assert(provided_thread_support >= MPI_THREAD_FUNNELED);

    if (num_threads <= 0) {
        hybrid_num_threads = omp_get_max_threads();
    } else {
        hybrid_num_threads = num_threads;
    }
    gtmp_init(hybrid_num_threads);

    // passing a dummy value here
    gtmpi_init(1);
}

void gtmp_hybrid_barrier(void) {
    // all threads arrive locally
    gtmp_barrier();

    // one representative thread per process will then participate in the MPI barrier
    if (omp_get_thread_num() == 0) {
        gtmpi_barrier();
    }

    // releasing all the rest of the local threads once MPI barrier completes
    gtmp_barrier();
}

void gtmp_hybrid_finalize(void) {

    #pragma omp parallel num_threads(hybrid_num_threads)
    {
        #pragma omp single
        {
            gtmp_finalize();
        }
    }

    gtmpi_finalize();
}