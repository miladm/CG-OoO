RESULT_IN_DIR=$1
RESULT_OUT_DIR=$RESULT_IN_DIR/analysis

rm -rf $RESULT_OUT_DIR
mkdir $RESULT_OUT_DIR

#./run.sh c13_s0_r0_m0 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c13_s0_r0_m2 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c13_s0_r1_m0 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c13_s0_r1_m2 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c13_s1_r0_m0 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c13_s1_r0_m2 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c13_s1_r1_m0 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c13_s1_r1_m2 $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c2_s0_r0_m2  $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c2_s0_r0_m0  $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c1_s0_r0_m0  $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c2_s1_r0_m2  $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
./run.sh c2_s1_r0_m0  $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
#./run.sh c1_s1_r0_m0  $RESULT_IN_DIR $RESULT_OUT_DIR 2>&1 | tee -a log.out
