#!/bin/bash
#
# Script for testing the correct results of epsilod and for getting its time measures. Adapted from script by Arturo Gonzalez Escribano.
#
# (c) 2024 Tomás de la Cal Esteban
#

# 1. EXPERIMENT INFORMATION
# 1.1. APP NAME AND DEFAULT EXECUTABLES LIST
APP_NAME="ALB Unit Testing TFG"
EXE_BASE="../../../build/ParallelStencilSkeleton"
DEVICE_FILE_PATH="./devicefiles"

# 1.2. APP/EXPERIMENT SPECIFIC PARAMETERS
TIMEOUT=7
CPUS_PER_TASK=16
PROCS=("1" "2" "8")
OUTPUT_DIR="results"
RESULT_FILE="Matrix.out.txt"


# 2. WRITE HEADER
echo
echo TESTING SCRIPT
echo --------------
echo "APPLICATION: $APP_NAME"

# 4. TESTING EXPERIMENT
function doTest() {
    #echo -e -n "Test $6 $4 $5 $3 $8:\t"
    #doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
    #doTest "$command" "$exe" "$procs" "$devicefile" "$expected"
    noextensiondevicefile=$(basename "$devicefile" .conf)


    resname="$2 $3 $5.res"
    errname="$2 $3 $5.err"

    resname_nospaces=$(echo $resname | sed 's/ /_/g')
    errname_nospaces=$(echo $errname | sed 's/ /_/g')

    rm -f $RESULT_FILE


    $1 -n $3 ./$EXE_BASE/$2 $4 >./$OUTPUT_DIR/$resname_nospaces 2>./$OUTPUT_DIR/$errname_nospaces

    resValue=$?

    # EXECUTION ERRORS, SKIP TESTING RESULT FILE
	if [ $5 != $resValue ]; then
		#echo ERROR EXECUTING: $(tail ./$OUTPUT_DIR/$errname_nospaces)
        echo -e -n "\tERROR $resValue"
        return -1
	fi

    echo -e -n "\tOK"
}



# SETUP ENV VARIABLES
export HIT_FILE_HEADER=no
export HIT_FILE_TEXT=yes
export TEST_EPSILOD_WRITE_OUTPUT=none
export HIT_FILE_TXT_SIZE=42
export HIT_FILE_TXT_DECIMALS=40
export OMP_NUM_THREADS=24

# SETUP RESULTS DIR

mkdir -p $OUTPUT_DIR


# 5. LOOPS FOR TESTS
devicefile="$DEVICE_FILE_PATH/allCPUs_final.conf"
machine="gorgon"
command="srun -w $machine --mpi=pmi2 --cpus-per-task=$CPUS_PER_TASK -Q --exclusive -t $TIMEOUT"

echo
echo "Testing: computeCommonData"
echo "------------------------"
echo -e -n "\t\t\tProcs: \t1 \t2 \t8\n\n"
echo -e -n "\t Testing Correct"
doTest "$command" "testing_computeCommonData" "1" "$devicefile" "20"
doTest "$command" "testing_computeCommonData" "2" "$devicefile" "20"
doTest "$command" "testing_computeCommonData" "8" "$devicefile" "20"
echo

echo -e -n "\t Testing InvShp\t"
doTest "$command" "testing_computeCommonData_InvShp" "1" "$devicefile" "15"
doTest "$command" "testing_computeCommonData_InvShp" "2" "$devicefile" "137"
doTest "$command" "testing_computeCommonData_InvShp" "8" "$devicefile" "137"
echo

echo -e -n "\t Testing InvBord"
doTest "$command" "testing_computeCommonData_InvShp" "1" "$devicefile" "15"
doTest "$command" "testing_computeCommonData_InvShp" "2" "$devicefile" "137"
doTest "$command" "testing_computeCommonData_InvShp" "8" "$devicefile" "137"
echo
echo



echo
echo "Testing: computeSingleTileData"
echo "------------------------"
echo -e -n "\t\t\tProcs: \t1 \t2 \t8\n\n"
echo -e -n "\t Testing Correct"
doTest "$command" "testing_computeSingleTileData" "1" "$devicefile" "20"
doTest "$command" "testing_computeSingleTileData" "2" "$devicefile" "20"
doTest "$command" "testing_computeSingleTileData" "8" "$devicefile" "20"
echo

echo -e -n "\t Testing InvShp\t"
doTest "$command" "testing_computeSingleTileData_InvShp" "1" "$devicefile" "15"
doTest "$command" "testing_computeSingleTileData_InvShp" "2" "$devicefile" "139"
doTest "$command" "testing_computeSingleTileData_InvShp" "8" "$devicefile" "139"
echo

echo -e -n "\t Testing InvBord"
doTest "$command" "testing_computeSingleTileData_InvShp" "1" "$devicefile" "15"
doTest "$command" "testing_computeSingleTileData_InvShp" "2" "$devicefile" "139"
doTest "$command" "testing_computeSingleTileData_InvShp" "8" "$devicefile" "139"
echo
echo



echo
echo "Testing: expandShapeBorder y expandShapeBordersAndHalos"
echo "------------------------"
echo -e -n "\t\t\tProcs: \t1 \t2 \t8\n\n"
echo -e -n "\t Testing Correct"
doTest "$command" "testing_expandBorders" "1" "$devicefile" "20"
doTest "$command" "testing_expandBorders" "2" "$devicefile" "20"
doTest "$command" "testing_expandBorders" "8" "$devicefile" "20"
echo
echo



echo
echo "Testing: selectIOTiles"
echo "------------------------"
echo -e -n "\t\t\tProcs: \t1 \t2 \t8\n\n"
echo -e -n "\t Testing Correct"
doTest "$command" "testing_selectIOTiles" "1" "$devicefile" "20"
doTest "$command" "testing_selectIOTiles" "2" "$devicefile" "20"
doTest "$command" "testing_selectIOTiles" "8" "$devicefile" "20"
echo
echo



echo
echo "Testing: hit_pattern"
echo "------------------------"
echo -e -n "\t\t\tProcs: \t1 \t2 \t8\n\n"
echo -e -n "\t Testing Correct"
doTest "$command" "testing_hit_pattern" "1" "$devicefile" "20"
doTest "$command" "testing_hit_pattern" "2" "$devicefile" "20"
doTest "$command" "testing_hit_pattern" "8" "$devicefile" "20"
echo
echo



echo
echo "Testing: alb"
echo "------------------------"
echo -e -n "\t\t\tProcs: \t1 \t2 \t8\n\n"
echo -e -n "\t Testing Correct"
doTest "$command" "testing_alb" "1" "$devicefile" "20"
doTest "$command" "testing_alb" "2" "$devicefile" "20"
doTest "$command" "testing_alb" "8" "$devicefile" "20"
echo
echo



# 6. CLEAN
rm -f $RESULT_FILE
rm -f check.err
#rm -rf CorrectResults