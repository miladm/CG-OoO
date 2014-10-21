#!/bin/sh

PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/out2'
BENCH_EXEC='/home/milad/esc_project/svn/PARS/benchmarks/ubench'
BENCH_INPUT=''
CFG_PATH='/home/milad/esc_project/svn/PARS/src/config'
CFG_FILE=$1
PIN_PARAM=-separate_memory  #-pin_memory_range 0x80000000:0x90000000 -pause_tool 15 

$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/ub_simple_cfg_loop -c $CFG_PATH/$CFG_FILE -- $BENCH_EXEC/ub_simple_cfg_loop
