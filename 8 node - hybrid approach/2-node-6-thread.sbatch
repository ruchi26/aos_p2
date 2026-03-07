#!/bin/bash

#SBATCH -J cs6210-proj2-combined-6
#SBATCH -N 2 --ntasks-per-node=1 --cpus-per-task=6
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o combined-2-6.out

echo "Started on `/bin/hostname`"

cd ~/combined

module load gcc/12.3.0 mvapich2/2.3.7-1
export OMP_NUM_THREADS=6

mpicc -o -Wall -lm -fopenmp -lgomp combined.c gtmp2.c gtmpi1.c -o combined

srun ./combined 6
