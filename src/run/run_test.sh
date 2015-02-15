#!/bin/sh

PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/out2'
BENCH_EXEC='/home/milad/zsim-apps/build/speccpu2006'
BENCH_INPUT='/home/milad/zsim-apps/inputs'
CFG_PATH=$PARS_ROOT'/config'
CFG_FILE=$1
PIN_PARAM='-separate_memory  1' #-pin_memory_range 0x80000000:0x90000000 -pause_tool 15 

echo "Copying the YAML config file to output dir"
cp $CFG_PATH/base.yaml $OUT_PATH/.

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/403.gcc -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/403.gcc/403.gcc  -c $BENCH_INPUT/403.gcc/test/input/cccp.i -o out.o

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/429.mcf -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/429.mcf/429.mcf $BENCH_INPUT/429.mcf/test/input/inp.in

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/401.bzip2 -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/401.bzip2/401.bzip2  $BENCH_INPUT/401.bzip2/test/input/dryer.jpg 2

# $PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/456.hmmer -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/456.hmmer/456.hmmer --fixed 0 --mean 325 --num 45000 --sd 200 --seed 0  $BENCH_INPUT/456.hmmer/test/input/bombesin.hmm

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/458.sjeng -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/458.sjeng/458.sjeng $BENCH_INPUT/458.sjeng/test/input/test.txt

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/445.gobmk -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/445.gobmk/445.gobmk --quiet --mode gtp < $BENCH_INPUT/445.gobmk/test/input/capture.tst

$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/462.libquantum -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/462.libquantum/462.libquantum 33 5

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/464.h264ref -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/464.h264ref/464.h264ref  -d $BENCH_INPUT/464.h264ref/test/input/foreman_test_encoder_baseline.cfg -p InputFile="$BENCH_INPUT/464.h264ref/all/input/foreman_qcif.yuv"

#cd $BENCH_INPUT/471.omnetpp/test/input
#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/471.omnetpp -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/471.omnetpp/471.omnetpp -d $BENCH_INPUT/471.omnetpp/test/input/omnetpp.ini

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/473.astar -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/473.astar/473.astar $BENCH_INPUT/473.astar/test/input/lake.cfg

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/483.xalancbmk -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/483.xalancbmk/483.xalancbmk -v $BENCH_INPUT/483.xalancbmk/test/input/test.xml $BENCH_INPUT/483.xalancbmk/test/input/xalanc.xsl

# $PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/400.perlbench -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/400.perlbench/400.perlbench -I $BENCH_INPUT/400.perlbench/ref/lib/test.xml $BENCH_INPUT/400.perlbench/test/input/attrs.pl

#$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/bzip2  -kf /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/sample2.ref
#echo "$PIN_ROOT/pin $PIN_PARAM -t $PARS_ROOT/obj-intel64/main_pars.so -b $CFG_PATH/462.libquantum -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/462.libquantum/462.libquantum 33 5"
