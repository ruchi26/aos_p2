#!/bin/bash

#SBATCH -J cs6210-proj2-mphello
#SBATCH -N 1 --cpus-per-task=6
#SBATCH --mem-per-cpu=1G
#SBATCH -t 120
#SBATCH -q coc-ice
#SBATCH -o mp-6-final.out

echo "Started on `/bin/hostname`"

cd ~/omp

module load gcc/12.3.0 mvapich2/2.3.7-1
export OMP_NUM_THREADS=6

gcc -c -std=gnu99 -I. -Wall -fopenmp harness.c -o harness.o
gcc -g -std=gnu99 -I. -Wall -fopenmp -o mp1 gtmp1.c harness.o -lgomp -lm 
gcc -g -std=gnu99 -I. -Wall -fopenmp -o mp2 gtmp2.c harness.o -lgomp -lm 
./mp1 6
./mp2 6