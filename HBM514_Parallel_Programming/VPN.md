
============RUNNING A JOB AT UHEM==============

# prepare / modify your submit.sl slurm file 
SBATCH -A projectName

# for getting project name → use projelerim command
SBATCH -n numberOfProcs

# enter a number of processor that is a multiple of 28
SBATCH -p queueName →  hbm513q

# assign a jobname with — job-name flag

# assign a value for the expected running time of your code

# Loading a module
module load mpi 
or openmpi or intel/mpi its your call

module load mpi/openmpi-3.1.6-gcc-8.3.0
# Compile the code before submiting the job via slurm

# Change the name of the executable and the output file that is written in slurm file
mpirun a.out > out

# Useful Commands while working at Uhem
projelerim
bosmakinalar

# Submitting a job
sbatch submit.sl

# Checking your jobs
squeue
isler # list of jobs

# If you know the id of your job, you can basically see the status of the job or the where the job is located in your home
scontrol show job jobID

# Cancel a job by using its ID
scancel jobID


# Installing Openmpi to your own Linux for the following distros Mxlinux - Ubuntu - Mint
sudo apt-get update -y
sudo apt-get install -y openmpi-bin
sudo apt install libopenmpi-dev/stable

# openmpi compilation
mpicc kod.c
mpicc ornek1SendRecv.c -o ornek1SendRecv.x (compiling your code by giving it a new name different than a.out)

#You can get intel compilers by using your e-mails (edu) Parallel Studio

# When you try to run your code with a number of cores that you dont actually have
# It may give the following error
--------------------------------------------------------------------------
There are not enough slots available in the system to satisfy the 20 slots
that were requested by the application:
--------------------------------------------------------------------------
# In this case, try it with the following flag
mpirun --oversubscribe -np 20 ./a.out

# For copying your files from your computer to Uhem after connecting Uhem with VPN
scp -r name_of_the_folder hbm51307@sariyer.uhem.itu.edu.tr:/okyanus/users/hbm51307/
name_of_the_folder = the name of the filevpncor folder that you want to copy
userName = it your user name that i sent via e-mail.

# For copying your files from Uhem to your computer after connecting Uhem with VPN
scp -r userName@sariyer.uhem.itu.edu.tr:/okyanus/users/userName/name_of_the_folder/* .

# Copying with rysnc
rsync -chavzP userName@sariyer.uhem.itu.edu.tr:/okyanus/users/userName/* .

VPN pwd: ciUeU9FkTr
UHEM user: hbm51307
UHEM pwd:  6Cb9b0GmcwkUG2