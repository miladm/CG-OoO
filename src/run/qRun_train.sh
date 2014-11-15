#!/bin/sh

OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/out1'
PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
BENCH_EXEC='/home/milad/zsim-apps/build/speccpu2006'
BENCH_INPUT='/home/milad/zsim-apps/inputs'
RUN_DIR='/home/milad/esc_project/svn/PARS/src/run'
CFG_PATH='/home/milad/esc_project/svn/PARS/src/config'
CFG_FILE=$1

mkdir $OUT_PATH

## QSUB
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_gcc        $RUN_DIR/run.sh -b $CFG_PATH/403.gcc        -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/403.gcc/403.gcc -C $BENCH_INPUT/403.gcc/train/input/integrate.i -o $OUT_PATH/out.o
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_mcf        $RUN_DIR/run.sh -b $CFG_PATH/429.mcf        -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/429.mcf/429.mcf $BENCH_INPUT/429.mcf/train/input/inp.in
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_bzip2      $RUN_DIR/run.sh -b $CFG_PATH/401.bzip2      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/401.bzip2/401.bzip2 $BENCH_INPUT/401.bzip2/train/input/byoudoin.jpg 5
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_hmmer      $RUN_DIR/run.sh -b $CFG_PATH/456.hmmer      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/456.hmmer/456.hmmer --fixed 0 --mean 425 --num 85000 --sd 300 --seed 0  $BENCH_INPUT/456.hmmer/train/input/leng100.hmm
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_sjeng      $RUN_DIR/run.sh -b $CFG_PATH/458.sjeng      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/458.sjeng/458.sjeng $BENCH_INPUT/458.sjeng/train/input/train.txt
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_gobmk      $RUN_DIR/run.sh -b $CFG_PATH/445.gobmk      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/445.gobmk/445.gobmk --quiet --mode gtp < $BENCH_INPUT/445.gobmk/train/input/arb.tst
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_libquantum $RUN_DIR/run.sh -b $CFG_PATH/462.libquantum -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/462.libquantum/462.libquantum 143 25
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_h264ref    $RUN_DIR/run.sh -b $CFG_PATH/464.h264ref    -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/464.h264ref/464.h264ref  -d $BENCH_INPUT/464.h264ref/train/input/foreman_train_encoder_baseline.cfg -p InputFile="$BENCH_INPUT/464.h264ref/all/input/foreman_qcif.yuv"
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_astar      $RUN_DIR/run.sh -b $CFG_PATH/473.astar      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/473.astar/473.astar $BENCH_INPUT/473.astar/train/input/BigLakes1024.cfg
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_xalancbmk  $RUN_DIR/run.sh -b $CFG_PATH/483.xalancbmk  -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/483.xalancbmk/483.xalancbmk -v $BENCH_INPUT/483.xalancbmk/train/input/allbooks.xml $BENCH_INPUT/483.xalancbmk/train/input/xalanc.xsl
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_perlbench  $RUN_DIR/run.sh -b $CFG_PATH/400.perlbench  -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/400.perlbench/400.perlbench -I $BENCH_INPUT/400.perlbench/all/input/lib/ $BENCH_INPUT/400.perlbench/train/input/scrabbl.pl $BENCH_INPUT/400.perlbench/train/input/scrabbl.in
                                                                                                                                                                               
cd $BENCH_INPUT/471.omnetpp/train/input
qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_omnetpp    $RUN_DIR/run.sh -b $CFG_PATH/400.perlbench  -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/471.omnetpp/471.omnetpp -d omnetpp.ini
