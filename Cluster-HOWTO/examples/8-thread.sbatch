#!/bin/bash

#SBATCH -J cs6210-proj2-mphello
#SBATCH -N 1 --cpus-per-task=8
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o mp2-8.out

echo "Started on `/bin/hostname`"

cd ~/omp

module load gcc/12.3.0 mvapich2/2.3.7-1
export OMP_NUM_THREADS=8

gcc -c -std=gnu99 -I. -Wall -fopenmp harness.c -o harness.o
gcc -g -std=gnu99 -I. -Wall -fopenmp -o mp2 gtmp2.c harness.o -lgomp -lm 
./mp2 8
