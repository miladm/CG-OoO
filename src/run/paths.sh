PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/out2'
BENCH_EXEC='/home/milad/zsim-apps/build/speccpu2006'
BENCH_INPUT='/home/milad/zsim-apps/inputs'
CFG_PATH=$PARS_ROOT'/config'
CFG_FILE=$1
PIN_PARAM=-separate_memory  1 #-pin_memory_range 0x80000000:0x90000000 -pause_tool 15 
