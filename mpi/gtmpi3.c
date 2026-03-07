#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"
#include <unistd.h> 
#include <sys/time.h>
#include <math.h>


static int rank, size;
static int sense = 1;
static int epoch = 0;
static char* role_array;

// message counters for measurement purposes
static long msg_send_count = 0, msg_recv_count = 0;

void gtmpi_init(int num_processes){
    
    if (num_processes < 1) {
        printf("Invalid number of processes\n");
        exit(1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    role_array = malloc((int)((ceil(log2(size))) + 1)* sizeof(char));
    for (int i = 0; i <= (int)(ceil(log2(size))); i++) {
        if (rank == 0) {
            if (i == 0) {
                role_array[i] = 'd';
                // printf("rank %d role_array[%d] %c\n", rank, i, role_array[i]);
                // fflush(stdout);
                continue;
            }
            role_array[i] = 'c';
            // printf("rank %d role_array[%d] %c\n", rank, i, role_array[i]);
            // fflush(stdout);
            continue;
        }

        if (i == 0) {
            role_array[i] = 'd';
        }
        else if ((rank % (int)pow(2, i) == 0) && ((rank + pow(2, i - 1)) < size)) {
            role_array[i] = 'w';
        }
        else if ((rank % (int)pow(2, i) == 0) && ((rank + pow(2, i - 1)) >= size)) {
            role_array[i] = 'b';
        }
        else if ((rank % (int)pow(2, i)) == pow(2, i - 1)) {
            role_array[i] = 'l';
        }
        else {
            role_array[i] = 'u';
        }
        // printf("rank %d role_array[%d] %c\n", rank, i, role_array[i]);
        // fflush(stdout);
    }
}

void gtmpi_barrier(){
    int msg = sense;
    MPI_Status status;

    for (int i = 0; i <= (int)(ceil(log2(size))); i++) {
        if (role_array[i] == 'd') {
            continue;
        }
        if (role_array[i] == 'l') {
            int dest = rank - (int)pow(2, i - 1);
            MPI_Send(&msg, 1, MPI_INT, dest, i, MPI_COMM_WORLD);
            msg_send_count++;

            int recv;
            MPI_Bcast(&recv, 1, MPI_INT, 0, MPI_COMM_WORLD);
            msg_recv_count++;

            break;
        }
        else if (role_array[i] == 'w') {
            int recv;
            int sender = rank + (int)pow(2, i - 1);
            MPI_Recv(&recv, 1, MPI_INT, sender, i, MPI_COMM_WORLD, &status);
            msg_recv_count++;
        }
        else if (role_array[i] == 'c') {
            int recv;
            int sender = (int)pow(2, i - 1);
            MPI_Recv(&recv, 1, MPI_INT, sender, i, MPI_COMM_WORLD, &status);
            msg_recv_count++;
            if (i == (int)(ceil(log2(size)))) {
                int bcast_msg = sense;
                MPI_Bcast(&bcast_msg, 1, MPI_INT, 0, MPI_COMM_WORLD);
                msg_send_count += size - 1;
            }
        }
    }
    
    sense = !sense;
    epoch++;

}

void gtmpi_get_and_reset_counts(long* sends, long* recvs) {
    if (sends) *sends = msg_send_count;
    if (recvs) *recvs = msg_recv_count;
    msg_send_count = 0;
    msg_recv_count = 0;
}

void gtmpi_finalize(){

    rank = MPI_UNDEFINED;
    size = 0;
    sense = 1;
    epoch = 0;

}