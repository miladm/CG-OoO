#!/bin/sh

#instructions: just uncomment the program for whiech you wish to create its static objdump from Pin (and the lines next to it)

FRONTEND_PATH="/home/milad/esc_project/svn/PARS/src/binaryTranslator/frontend"

# ####
# ../../../../pin/pin -t ./obj-intel64/parser.so -- /home/milad/zsim-apps/build/speccpu2006/429.mcf/429.mcf ~/zsim-apps/inputs/429.mcf/test/input/inp.in
# mv static_trace.s ../input_files/429.mcf.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/429.mcf_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/429.mcf_ins_addrs.csv
#
 ####
  cd /home/milad/zsim-apps/build/speccpu2006/400.perlbench
  /home/milad/esc_project/svn/pin/pin -t $FRONTEND_PATH/obj-intel64/parser.so -- ./400.perlbench -I. -I /home/milad/zsim-apps/inputs/400.perlbench/all/input/lib /home/milad/zsim-apps/inputs/400.perlbench/test/input/attrs.pl
  cd $FRONTEND_PATH
  mv static_trace.s ../input_files/400.perlbench.s
  mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/400.perlbench_mem_trace.csv
  mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/400.perlbench_ins_addrs.csv
#
#####
# ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/403.gcc/403.gcc -C ~/zsim-apps/inputs/403.gcc/test/input/cccp.i  -o junk.o
# mv static_trace.s ../input_files/403.gcc.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/403.gcc_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/403.gcc_ins_addrs.csv
#
#####
# ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/445.gobmk/445.gobmk --quiet --mode gtp <~/zsim-apps/inputs/445.gobmk/test/input/capture.tst
# mv static_trace.s ../input_files/445.gobmk.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/445.gobmk_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/445.gobmk_ins_addrs.csv
#
#####
# ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/456.hmmer/456.hmmer --fixed 0 --mean 325 --num 45000 --sd 200 --seed 0 ~/zsim-apps/inputs/456.hmmer/test/input/bombesin.hmm
# mv static_trace.s ../input_files/456.hmmer.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/456.hmmer_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/456.hmmer_ins_addrs.csv
#
#####
# ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/458.sjeng/458.sjeng ~/zsim-apps/inputs/458.sjeng/test/input/test.txt
# mv static_trace.s ../input_files/458.sjeng.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/458.sjeng_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/458.sjeng_ins_addrs.csv

###
 ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/462.libquantum/462.libquantum 33 5
 mv static_trace.s ../input_files/462.libquantum.s
 mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/462.libquantum_mem_trace.csv
 mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/462.libquantum_ins_addrs.csv

###
 ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/464.h264ref/464.h264ref -d ~/zsim-apps/inputs/464.h264ref/test/input/foreman_test_encoder_baseline.cfg -p InputFile=/home/milad/zsim-apps/inputs/464.h264ref/all/input/foreman_qcif.yuv
 mv static_trace.s ../input_files/464.h264ref.s
 mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/464.h264ref_mem_trace.csv
 mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/464.h264ref_ins_addrs.csv

#####
# cd /home/milad/zsim-apps/build/speccpu2006/471.omnetpp
# /home/milad/esc_project/svn/pin/pin -t $FRONTEND_PATH/obj-intel64/parser.so -- ./471.omnetpp /home/milad/zsim-apps/inputs/471.omnetpp/test/input/omnetpp.ini
# cd $FRONTEND_PATH
# mv static_trace.s ../input_files/471.omnetpp.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/471.omnetpp_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/471.omnetpp_ins_addrs.csv
#
#####
# ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/473.astar/473.astar ~/zsim-apps/inputs/473.astar/test/input/lake.cfg
# mv static_trace.s ../input_files/473.astar.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/473.astar_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/473.astar_ins_addrs.csv
#
#####
# ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/483.xalancbmk/483.xalancbmk -v ~/zsim-apps/inputs/483.xalancbmk/test/input/test.xml ~/zsim-apps/inputs/483.xalancbmk/test/input/xalanc.xsl
# mv static_trace.s ../input_files/483.xalancbmk.s
# mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/483.xalancbmk_mem_trace.csv
# mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/483.xalancbmk_ins_addrs.csv

#####
 ../../../../pin/pin -t ./obj-intel64/parser.so -- ~/zsim-apps/build/speccpu2006/401.bzip2/401.bzip2 ~/zsim-apps/inputs/401.bzip2/test/input/dryer.jpg 2
 mv static_trace.s ../input_files/401.bzip2.s
 mv /scratch/tracesim/specint2006/mem_trace.csv /scratch/tracesim/specint2006/401.bzip2_mem_trace.csv
 mv /scratch/tracesim/specint2006/ins_addrs.csv /scratch/tracesim/specint2006/401.bzip2_ins_addrs.csv



#####################################
#../../../../pin/pin -t ./obj-intel64/parser.so -- ../../../../benchmarks/spec2006/bzip2/bzip2 ../../../../benchmarks/spec2006/bzip2/sample2.ref
