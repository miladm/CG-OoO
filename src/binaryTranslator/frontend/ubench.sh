#!/bin/sh

FRONTEND_PATH="/home/milad/esc_project/svn/PARS/src/binaryTranslator/frontend"
UBENCH_PATH="/home/milad/esc_project/svn/PARS/benchmarks/ubench"

UBENCH_NAME="ub_simple_cfg_loop"
 ../../../../pin/pin -t ./obj-intel64/parser.so -- $UBENCH_PATH/$UBENCH_NAME
 mv static_trace.s ../input_files/${UBENCH_NAME}.s
 mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/${UBENCH_NAME}_mem_trace.csv
 mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/${UBENCH_NAME}_ins_addrs.csv
