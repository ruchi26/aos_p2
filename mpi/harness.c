#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "gtmpi.h"


void gtmpi_get_and_reset_counts(long* sends, long* recvs);

int main(int argc, char** argv)
{
  int num_processes, my_rank, num_rounds = 10;

  MPI_Init(&argc, &argv);
  
  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_PROCS]\n");
    exit(EXIT_FAILURE);
  }

  num_processes = strtol(argv[1], NULL, 10);

  gtmpi_init(num_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  //Measurement variables
  double lat_sum = 0.0, lat_min = 1e300, lat_max = 0.0;
  double wait_sum = 0.0, wait_min = 1e300, wait_max = 0.0;

  int k;
  for(k = 0; k < num_rounds; k++){
    // "round start" time 
    double round_start_local = 0.0;
    if (my_rank == 0) round_start_local = MPI_Wtime();
    MPI_Bcast(&round_start_local, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // each process's arrival time right before calling the barrier.
    double arrival_time = MPI_Wtime();
    double wait_before_barrier = arrival_time - round_start_local;

    double t0 = MPI_Wtime();
    gtmpi_barrier();
    double t1 = MPI_Wtime();
    
    double latency = t1 - t0;
    lat_sum += latency;
    if (latency < lat_min) lat_min = latency;
    if (latency > lat_max) lat_max = latency;

    wait_sum += wait_before_barrier;
    if (wait_before_barrier < wait_min) wait_min = wait_before_barrier;
    if (wait_before_barrier > wait_max) wait_max = wait_before_barrier;
  }

  
  // Aggregate latency stats at rank 0
  double g_lat_sum = 0.0, g_lat_min = 0.0, g_lat_max = 0.0;
  MPI_Reduce(&lat_sum, &g_lat_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&lat_min, &g_lat_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  MPI_Reduce(&lat_max, &g_lat_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // Aggregate wait-time stats at rank 0
  double g_wait_sum = 0.0, g_wait_min = 0.0, g_wait_max = 0.0;
  MPI_Reduce(&wait_sum, &g_wait_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&wait_min, &g_wait_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  MPI_Reduce(&wait_max, &g_wait_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  long local_sends = 0, local_recvs = 0, sum_sends = 0, sum_recvs = 0;
  gtmpi_get_and_reset_counts(&local_sends, &local_recvs);
  MPI_Reduce(&local_sends, &sum_sends, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&local_recvs, &sum_recvs, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);


  if (my_rank == 0) {
    
    // Average per barrier over all ranks
    double avg_latency = g_lat_sum / num_processes / num_rounds;
    double avg_wait    = g_wait_sum / num_processes / num_rounds;

    // Print lines easy to parse downstream
    printf("Latency , P=%d, ROUNDS=%d, AVG_LAT=%.9f, MIN_LAT=%.9f, MAX_LAT=%.9f\n",
            num_processes, num_rounds, avg_latency, g_lat_min, g_lat_max);

    printf("Wait Time, P=%d, ROUNDS=%d, AVG_WAIT=%.9f, MIN_WAIT=%.9f, MAX_WAIT=%.9f\n",
            num_processes, num_rounds, avg_wait, g_wait_min, g_wait_max);

    double sends_per_barrier = (double)sum_sends / (double)num_rounds;
    double recvs_per_barrier = (double)sum_recvs / (double)num_rounds;
    printf("Network Message exchanges count, Process Count=%d, ROUNDS=%d, SENDS_PER_BARRIER=%.2f, RECVS_PER_BARRIER=%.2f\n",
            num_processes, num_rounds, sends_per_barrier, recvs_per_barrier);
    fflush(stdout);
  }


  gtmpi_finalize();  

  MPI_Finalize();

  return 0;
}
