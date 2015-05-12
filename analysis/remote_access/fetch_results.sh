SERVER="milad@snooki.stanford.edu"
RESULTS_DIR="/scratch/milad/qsub_outputs/perf_sim_test/"
EXT=".csv"

#EXP="multi_bb_fetch/width_size/width_1/o3_ino"
#EXP="multi_bb_fetch/width_size/width_1/bb"
EXP='multi_bb_fetch/perfect_mem/o3_ino_w1'


FILE="commit.ipc"
scp $SERVER:$RESULTS_DIR/$EXP/analysis/$FILE$EXT .

FILE='Energy'
scp "$SERVER:$RESULTS_DIR/$EXP/analysis/$FILE$EXT" .
