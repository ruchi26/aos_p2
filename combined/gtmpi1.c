#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include "gtmpi.h"
#include <unistd.h> 
#include <time.h> 


static int totalProcessCount;
static int my_rank;
static int num_rounds;

void gtmpi_init(int num_processes){
    if (num_processes < 1) {
        printf("Invalid number of processes\n");
        exit(1);
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcessCount);
    
    //number of rounds needed
    num_rounds = (int)ceil(log2((double)totalProcessCount));
}

void gtmpi_barrier(){
    int round;
    int send_to;
    int recv_from;
    int distance;
    MPI_Status status;
    int sendbuffer = 1, recvbuffer = 0;

    // //adding random delays based on rank to check the barrier (all of them should leave only after the last process's entry time)
    // usleep(my_rank * 200 * 1000);   
    double t_enter = MPI_Wtime();
    fprintf(stderr, "[%d] before barrier at %.6f\n", my_rank, t_enter);

    for (round = 0; round < num_rounds; round++) {
        distance = 1 << round;
        
        // Process i with id my_rank will send to (my_rank + 2^round) mod n
        send_to = (my_rank + distance) % totalProcessCount;
        
        // Process i with id my_rank will receive from (my_rank - 2^round) mod n
        recv_from = (my_rank - distance + totalProcessCount) % totalProcessCount;
        
        MPI_Sendrecv(&sendbuffer, 1, MPI_INT, send_to, round,
                    &recvbuffer, 1, MPI_INT, recv_from, round,
                    MPI_COMM_WORLD, &status);
    }

    double t_leave = MPI_Wtime();
    fprintf(stderr, "[%d] after barrier at %.6f\n",
            my_rank, t_leave);
}

void gtmpi_finalize(){

    totalProcessCount = 0;
    my_rank = MPI_UNDEFINED;
    num_rounds = 0;

}
