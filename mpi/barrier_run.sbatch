#!/bin/bash
#
# SLURM directives
#
#SBATCH -J cs6210-proj2-barriers
#SBATCH -N 12                 # request up to 12 nodes; actual srun size is set per run below
#SBATCH --ntasks-per-node=1   # 1 MPI rank per node
#SBATCH --mem-per-cpu=1G
#SBATCH -t 00:15:00           # enough for all runs in sequence
#SBATCH -q coc-ice
#SBATCH -o slurm_barriers_%j.out
#SBATCH -e slurm_barriers_%j.err

echo "Job $SLURM_JOB_ID started on $(/bin/hostname) at $(date)"

# ----- User-configurable knobs -----
ROUNDS=10              # number of measured rounds per run (argv[2] to harness)
OUTDIR=~/out   # where results will be written
P_START=2
P_END=12
# -----------------------------------
mkdir -p "${OUTDIR}"
module purge
module load gcc/12.3.0 mvapich2/2.3.7-1

cd ~/mpi  || { echo "build failed: cannot change directory"; exit 2; }

# Clean & build both variants:
# - Dissemination barrier  -> harness_dissemination
# - Tree barrier           -> harness_tree
#
# NOTE: gtmpi1.c uses math (ceil/log2), so link -lm.
mpicc -O3 -Wall harness.c gtmpi1.c -lm -o harness_dissemination || { echo "build failed: dissemination"; exit 2; }
mpicc -O3 -Wall harness.c gtmpi2.c      -o harness_tree          || { echo "build failed: tree"; exit 3; }

echo "Binaries built:"
ls -lh harness_dissemination harness_tree

# Sweep P = 2..12, one rank per node
for P in $(seq ${P_START} ${P_END}); do
    echo "=== Running P=${P} (dissemination) ==="
    # Output files for this P
    OUT_DIS="${OUTDIR}/dissemination_P${P}.txt"
    # Run: -N P nodes, -n P tasks, 1 task per node (already set via --ntasks-per-node=1 in the job)
    # Use srun so the allocation is handled by SLURM within this single job.
    srun --nodes=${P} --ntasks=${P} --ntasks-per-node=1 ./harness_dissemination ${P} ${ROUNDS} | tee "${OUT_DIS}"

    echo "=== Running P=${P} (tree) ==="
    OUT_TREE="${OUTDIR}/tree_P${P}.txt"
    srun --nodes=${P} --ntasks=${P} --ntasks-per-node=1 ./harness_tree ${P} ${ROUNDS} | tee "${OUT_TREE}"

    echo "Completed P=${P} at $(date)"
done

echo "Job $SLURM_JOB_ID finished at $(date)"