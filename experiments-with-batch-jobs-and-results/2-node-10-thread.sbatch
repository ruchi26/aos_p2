#!/bin/bash

#SBATCH -J cs6210-proj2-combined-10
#SBATCH -N 2 --ntasks-per-node=1 --cpus-per-task=10
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o combined-2-10.out

echo "Started on `/bin/hostname`"

cd ~/combined

module load gcc/12.3.0 mvapich2/2.3.7-1
export OMP_NUM_THREADS=10

mpicc -o -Wall -lm -fopenmp -lgomp combined.c gtmp2.c gtmpi1.c -o combined

srun ./combined 10
