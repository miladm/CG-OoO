SERVER=milad@snooki.stanford.edu
RESULTS_DIR="/scratch/milad/qsub_outputs/perf_sim_test"

EXP=multi_bb_fetch/backend_cfg/bb1
scp $SERVER:$RESULTS_DIR/$EXP/analysis/commit.ipc .
scp $SERVER:$RESULTS_DIR/$EXP/analysis/TOTAL.ipc .
