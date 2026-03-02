#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"
#include <unistd.h>   // for usleep()
#include <stdlib.h>   // for rand()
#include <time.h>     // for time()


static int rank, size;
static int parent;
static int child[2];
static int num_children;
static int sense = 1;
static int epoch = 0;


void gtmpi_init(int num_processes){

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        parent = -1; //this is the root, so no parent
    }
    else 
    {
        parent = (rank - 1) / 2;
    }

    num_children = 0;
    int left  = 2 * rank + 1;
    int right = 2 * rank + 2;

    if (left  < size) child[num_children++] = left;
    if (right < size) child[num_children++] = right;

}

void gtmpi_barrier(){

    int tag = epoch;       // each barrier has its own tag
    int msg = sense;
    MPI_Status status;

    // //adding random delays based on rank to check the barrier (all of them should leave only after the last process's entry time)
    // usleep(rank * 200 * 7000);   
    // double t_enter = MPI_Wtime();
    // fprintf(stderr, "[%d] before barrier at %.6f\n", rank, t_enter);

    // waiting for all children to receive
    for (int i = 0; i < num_children; i++) {
        int receiveBuffer;
        MPI_Recv(&receiveBuffer, 1, MPI_INT, child[i], tag, MPI_COMM_WORLD, &status);
    }

    // sending messages to all parents except the root, whose parent is -1
    if (parent != -1) {
        MPI_Send(&msg, 1, MPI_INT, parent, tag, MPI_COMM_WORLD);

        int receiveBuffer;
        MPI_Recv(&receiveBuffer, 1, MPI_INT, parent, tag, MPI_COMM_WORLD, &status);
    }

    for (int i = 0; i < num_children; i++) {
        MPI_Send(&msg, 1, MPI_INT, child[i], tag, MPI_COMM_WORLD);
    }

    // double t_leave = MPI_Wtime();
    // fprintf(stderr, "[%d] after barrier at %.6f\n",
    //         rank, t_leave);

    sense = !sense;
    epoch++;

}

void gtmpi_finalize(){

    rank = MPI_UNDEFINED;
    size = 0;
    parent = -1;
    child[0] = child[1] = -1;
    num_children = 0;
    sense = 1;
    epoch = 0;

}
