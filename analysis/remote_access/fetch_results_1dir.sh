SERVER="milad@snooki.stanford.edu"
EXT=".csv"

RESULTS_DIR="/scratch/milad/qsub_outputs/perf_sim_test"
TARGET_DIR="/multi_bb_fetch/multi_pass_issue/runahead_5"

SUB=blk
rm -rf $SUB
mkdir $SUB
EXP=$TARGET_DIR
scp $SERVER:$RESULTS_DIR/$EXP/analysis/* ./$SUB/.
echo $SERVER:$RESULTS_DIR/$EXP/analysis/
