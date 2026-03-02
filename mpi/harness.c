#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "gtmpi.h"

int main(int argc, char** argv)
{
  int num_processes, my_rank, num_rounds = 1;

  MPI_Init(&argc, &argv);
  
  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_PROCS]\n");
    exit(EXIT_FAILURE);
  }

  num_processes = strtol(argv[1], NULL, 10);

  gtmpi_init(num_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  int k;
  for(k = 0; k < num_rounds; k++){
    printf("process %d entering barrier %d\n", my_rank, k);
    fflush(stdout);
    gtmpi_barrier();
    printf("process %d left barrier %d\n", my_rank, k);
    fflush(stdout);
  }

  gtmpi_finalize();  

  MPI_Finalize();

  return 0;
}
