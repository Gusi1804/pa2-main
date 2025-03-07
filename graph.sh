#!/bin/bash

#SBATCH --job-name=mpi_results
#SBATCH --output=mpi_results.out
#SBATCH --error=mpi_results.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24
#SBATCH --time=00:05:30

mkdir -p autograderout

MAX_EXECUTION_TIME=300

# Run the tests within the time limit
timeout $MAX_EXECUTION_TIME bash -c '

lscpu
echo "-------------------------------------"

module load openmpi

make clean
make many2many
make allreduce

echo " "
echo "=================== CHECKING TEST FILE ==================="
echo " "

if [ ! -f "autograderdata/inputard_15.txt" ]; then
    echo "No AllReduce 15 Input"
    exit 1
fi

if [ ! -f "autograderdata/inputm2_15.txt" ]; then
    echo "No M2M 15 Input"
    exit 1
fi

if [ ! -f "autograderdata/inputard_24.txt" ]; then
    echo "No AllReduce 24 Input"
    exit 1
fi

if [ ! -f "autograderdata/inputm2_24.txt" ]; then
    echo "No M2M 24 Input"
    exit 1
fi

if [ ! -f "autograderdata/outputard_15.txt" ]; then
    echo "No AllReduce 15 Output"
    exit 1
fi

if [ ! -f "autograderdata/outputm2_15.txt" ]; then
    echo "No M2M 15 Output"
    exit 1
fi

if [ ! -f "autograderdata/outputard_24.txt" ]; then
    echo "No AllReduce 24 Output"
    exit 1
fi

if [ ! -f "autograderdata/outputm2_24.txt" ]; then
    echo "No M2M 24 Output"
    exit 1
fi


echo " "
echo "=================== CORRECTNESS TESTS ==================="
echo " "

echo "##AllReduce 4"
output=$(srun -n 4 ./ard autograderdata/inputard_4.txt autograderout/myoutputard_4.txt)
echo "$output"
if diff autograderdata/outputard_4.txt autograderout/myoutputard_4.txt > /dev/null; then
    echo "AllReduce P=4 Correctness passed!"
else
    echo "AllReduce P=4 Failed"
fi
echo " "

echo "##AllReduce 8"
output=$(srun -n 8 ./ard autograderdata/inputard_8.txt autograderout/myoutputard_8.txt)
echo "$output"
if diff autograderdata/outputard_8.txt autograderout/myoutputard_8.txt > /dev/null; then
    echo "AllReduce P=8 Correctness passed!"
else
    echo "AllReduce P=8 Failed"
fi
echo " "

echo "##AllReduce 16"
output=$(srun -n 16 ./ard autograderdata/inputard_16.txt autograderout/myoutputard_16.txt)
echo "$output"
if diff autograderdata/outputard_16.txt autograderout/myoutputard_16.txt > /dev/null; then
    echo "AllReduce P=16 Correctness passed!"
else
    echo "AllReduce P=16 Failed"
fi
echo " "

echo "##AllReduce 24"
output=$(srun -n 24 ./ard autograderdata/inputard_24.txt autograderout/myoutputard_24.txt)
echo "$output"

if diff autograderdata/outputard_24.txt autograderout/myoutputard_24.txt > /dev/null; then
    echo "AllReduce P=24 Correctness passed!"
else
    echo "AllReduce P=24 Failed"
fi
echo " "


echo " "
echo "=================================="
echo " "

echo "##Many2Many 4"
output=$(srun -n 4 ./m2m autograderdata/inputm2_4.txt autograderdata/outputm2_4.txt)
echo "$output"
echo " "

echo "##Many2Many 8"
output=$(srun -n 8 ./m2m autograderdata/inputm2_8.txt autograderdata/outputm2_8.txt)
echo "$output"
echo " "

echo "##Many2Many 16"
output=$(srun -n 16 ./m2m autograderdata/inputm2_16.txt autograderdata/outputm2_16.txt)
echo "$output"
echo " "

echo "##Many2Many 24"
output=$(srun -n 24 ./m2m autograderdata/inputm2_24.txt autograderdata/outputm2_24.txt)
echo "$output"