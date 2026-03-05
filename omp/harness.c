#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "gtmp.h"
#include <sys/time.h>

int print_order;

int main(int argc, char** argv)
{
  int num_threads, num_iter=10;

  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_THREADS]\n");
    exit(EXIT_FAILURE);
  }
  num_threads = strtol(argv[1], NULL, 10);

  omp_set_dynamic(0);
  if (omp_get_dynamic())
    printf("Warning: dynamic adjustment of threads has been set\n");

  omp_set_num_threads(num_threads);
  
  gtmp_init(num_threads);

#pragma omp parallel shared(num_threads)
   {
     int i;
     struct timeval time_curr_start;
     struct timeval time_curr_end;
     if (omp_get_thread_num() == 0) {
      gettimeofday(&time_curr_start, NULL);
     }
     for(i = 0; i < num_iter; i++){
       gtmp_barrier();
       if (omp_get_thread_num() == 0) {
        gettimeofday(&time_curr_end, NULL);
        double t_enter = (time_curr_end.tv_sec + time_curr_end.tv_usec / 1000000.0) - (time_curr_start.tv_sec + time_curr_start.tv_usec / 1000000.0);
        printf("time for barrier %d is %.9f\n", i, t_enter);
        gettimeofday(&time_curr_start, NULL);
       }
     }
   }

   gtmp_finalize();

   return 0;
}
