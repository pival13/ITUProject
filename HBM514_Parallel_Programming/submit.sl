#!/bin/bash
#SBATCH -A hbm51307
#SBATCH -n 1
#SBATCH -p hbm513q
#SBATCH --job-name "GS_1" 
#SBATCH -t 0-10:00

module load mpi/openmpi-3.1.6-gcc-8.3.0
mpirun GaussSeidel > out_GS_1p
