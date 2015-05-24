SERVER="milad@snooki.stanford.edu"
EXT=".csv"

RESULTS_DIR="/scratch/milad/qsub_outputs/perf_sim_test"
TARGET_DIR="/multi_bb_fetch/width_size/width_4"

SUB=o3_ino
rm -rf $SUB
mkdir $SUB
EXP=$TARGET_DIR/$SUB
scp $SERVER:$RESULTS_DIR/$EXP/analysis/* ./$SUB/.
echo $SERVER:$RESULTS_DIR/$EXP/analysis/

SUB=bb
rm -rf $SUB
mkdir $SUB
EXP=$TARGET_DIR/$SUB
scp $SERVER:$RESULTS_DIR/$EXP/analysis/* ./$SUB/.
echo $SERVER:$RESULTS_DIR/$EXP/analysis/
