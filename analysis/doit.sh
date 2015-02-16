RESULTS_DIR=/scratch/milad/qsub_outputs/perf_sim_test

# REMOVE OLD LOG FILE
rm log.out

./call_run.sh $RESULTS_DIR/multi_bb_fetch/backend_cfg/bb1
./call_run.sh $RESULTS_DIR/multi_bb_fetch/backend_cfg/bb2
./call_run.sh $RESULTS_DIR/multi_bb_fetch/backend_cfg/bb3
./call_run.sh $RESULTS_DIR/multi_bb_fetch/backend_cfg/bb3_4eu
./call_run.sh $RESULTS_DIR/multi_bb_fetch/backend_cfg/bb3_4eu_share2
./call_run.sh $RESULTS_DIR/multi_bb_fetch/backend_cfg/bb3_4eu_share4
