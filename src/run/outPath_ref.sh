#!/bin/sh

#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbWin_size/10_ins'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbWin_port/8_port'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbWin_cnt/16_bb'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbROB_size/64_bb'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/perfect_mem'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue/superblock'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue/wide_frontend'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/rf_size/100_pr'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/fu_size/fu_2'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/out1'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/backend_cfg/bb1'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue/runahead'
#./qRun_ref.sh $OUT_PATH $@
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue/no_runahead'
#./qRun_ref.sh $OUT_PATH $@
OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/locGlb_reg_alloc'
mkdir $OUT_PATH
./run/qRun_ref.sh $OUT_PATH $@
