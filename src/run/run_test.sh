#!/bin/sh

PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
OUT_PATH="/scratch/milad/qsub_outputs/wront_path/out2"

##@ TERMINAL
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/403.gcc/403.gcc -C /home/milad/zsim-apps/inputs/403.gcc/test/input/cccp.i -o out.o
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/esc_project/svn/benchmarks/spec2006/mcf-1.3_2/src/mcf /home/milad/esc_project/svn/benchmarks/spec2006/mcf-1.3_2/src/inp_small.in
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/bzip2  -kf /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/sample2.ref
 $PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/456.hmmer/456.hmmer --fixed 0 --mean 325 --num 45000 --sd 200 --seed 0  /home/milad/zsim-apps/inputs/456.hmmer/test/input/bombesin.hmm
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/458.sjeng/458.sjeng /home/milad/zsim-apps/inputs/458.sjeng/test/input/test.txt
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/445.gobmk/445.gobmk --quiet --mode gtp < /home/milad/zsim-apps/inputs/445.gobmk/test/input/capture.tst
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/462.libquantum/462.libquantum 33 5
#$PIN_ROOT/pin	-separate_memory -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/464.h264ref/464.h264ref  -d /home/milad/zsim-apps/inputs/464.h264ref/test/input/foreman_test_encoder_baseline.cfg -p InputFile="/home/milad/zsim-apps/inputs/464.h264ref/all/input/foreman_qcif.yuv"
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/471.omnetpp/471.omnetpp -d /home/milad/zsim-apps/inputs/471.omnetpp/test/input/omnetpp.ini
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/473.astar/473.astar /home/milad/zsim-apps/inputs/473.astar/test/input/lake.cfg
#$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/483.xalancbmk/483.xalancbmk -v /home/milad/zsim-apps/inputs/483.xalancbmk/test/input/test.xml /home/milad/zsim-apps/inputs/483.xalancbmk/test/input/xalanc.xsl
# $PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/400.perlbench/400.perlbench -I /home/milad/zsim-apps/inputs/400.perlbench/ref/lib/test.xml /home/milad/zsim-apps/inputs/400.perlbench/test/input/attrs.pl

#$PIN_ROOT/pin -pin_memory_range 0x80000000:0x90000000 -pause_tool 15 -t $PARS_ROOT/obj-intel64/main_pars.so  -usectxt -- /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/bzip2  -kf /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/sample2.ref

echo "$PIN_ROOT/pin -separate_memory -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/462.libquantum/462.libquantum 33 5"
