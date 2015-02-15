SERVER="milad@snooki.stanford.edu"
RESULTS_DIR="/scratch/milad/qsub_outputs/perf_sim_test"
EXT=".csv"

EXP=multi_bb_fetch/backend_cfg/bb1
scp $SERVER:$RESULTS_DIR/$EXP/analysis/commit.ipc$EXT .
scp $SERVER:$RESULTS_DIR/$EXP/analysis/TOTAL$EXT .
