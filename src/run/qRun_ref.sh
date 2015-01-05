#!/bin/sh

#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbWin_size/10_ins'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbWin_port/8_port'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbWin_cnt/16_bb'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/bbROB_size/64_bb'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/perfect_mem'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue'
OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue/5_ins'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/multi_pass_issue/wide_frontend'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/rf_size/100_pr'
#OUT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/multi_bb_fetch/fu_size/fu_2'
QSUB_OUT_PATH=$OUT_PATH/'qsub_files/'
PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
RUN_DIR='/home/milad/esc_project/svn/PARS/src/run'
BENCH_EXEC='/home/milad/zsim-apps/build/speccpu2006'
BENCH_INPUT='/home/milad/zsim-apps/inputs'
CFG_PATH='/home/milad/esc_project/svn/PARS/src/config'
CFG_FILE=$1
FILE=$OUT_PATH/README

mkdir $OUT_PATH
mkdir $QSUB_OUT_PATH

echo "Copying the YAML config file to output dir"
cp $CFG_PATH/base.yaml $OUT_PATH/.

###########################################
# ADDING README FILE
###########################################
 NOW=$(date +"%Y_%m_%d")
 if [ -f $FILE ];
 then
	echo "File \"$FILE\" exists. Rewriting."
	echo "Milad Mohammadi" > $FILE
	echo $NOW >> $FILE
	echo "---------------" >> $FILE
	echo $@ >> $FILE
 else
	echo "File \"$FILE\" does not exist. Creating it..."
	touch $FILE
	echo "Milad Mohammadi" > $FILE
	echo $NOW >> $FILE
	echo "---------------" >> $FILE
	echo $@ >> $FILE
 fi
###########################################

## QSUB
qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_gcc        $RUN_DIR/run.sh -b $CFG_PATH/403.gcc        -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/403.gcc/403.gcc -C $BENCH_INPUT/403.gcc/ref/input/scilab.i -o $QSUB_OUT_PATH/gcc_out.o
qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_mcf        $RUN_DIR/run.sh -b $CFG_PATH/429.mcf        -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/429.mcf/429.mcf $BENCH_INPUT/429.mcf/ref/input/inp.in
#qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_bzip2      $RUN_DIR/run.sh -b $CFG_PATH/401.bzip2      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/401.bzip2/401.bzip2 $BENCH_INPUT/401.bzip2/ref/input/input.source 64
qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_hmmer      $RUN_DIR/run.sh -b $CFG_PATH/456.hmmer      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/456.hmmer/456.hmmer $BENCH_INPUT/456.hmmer/ref/input/nph3.hmm $BENCH_INPUT/456.hmmer/ref/input/swiss41
qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_sjeng      $RUN_DIR/run.sh -b $CFG_PATH/458.sjeng      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/458.sjeng/458.sjeng $BENCH_INPUT/458.sjeng/ref/input/ref.txt
#qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_gobmk      $RUN_DIR/run.sh -b $CFG_PATH/445.gobmk      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/445.gobmk/445.gobmk --quiet --mode gtp < $BENCH_INPUT/445.gobmk/ref/input/13x13.tst
qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_libquantum $RUN_DIR/run.sh -b $CFG_PATH/462.libquantum -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/462.libquantum/462.libquantum 1397 8
#qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_h264ref    $RUN_DIR/run.sh -b $CFG_PATH/464.h264ref    -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/464.h264ref/464.h264ref  -d $BENCH_INPUT/464.h264ref/ref/input/foreman_ref_encoder_baseline.cfg -p InputFile="$BENCH_INPUT/464.h264ref/all/input/foreman_qcif.yuv"
qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_astar      $RUN_DIR/run.sh -b $CFG_PATH/473.astar      -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/473.astar/473.astar $BENCH_INPUT/473.astar/ref/input/BigLakes2048.cfg
#qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_xalancbmk  $RUN_DIR/run.sh -b $CFG_PATH/483.xalancbmk  -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/483.xalancbmk/483.xalancbmk -v $BENCH_INPUT/483.xalancbmk/ref/input/t5.xml $BENCH_INPUT/483.xalancbmk/ref/input/xalanc.xsl
qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_perlbench  $RUN_DIR/run.sh -b $CFG_PATH/400.perlbench  -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/400.perlbench/400.perlbench -I $BENCH_INPUT/400.perlbench/all/input/lib/ $BENCH_INPUT/400.perlbench/ref/input/checkspam.pl 2500 5 25 11 150 1 1 1 1
#
#cd "$BENCH_ROOT/471.omnetpp/ref/input/"
#echo "$BENCH_ROOT/471.omnetpp/ref/input/"
#qsub -e $QSUB_OUT_PATH -o $QSUB_OUT_PATH -V -l p=16 -pe smp 1 -S /bin/sh -N re_omnetpp    $RUN_DIR/run.sh -b $CFG_PATH/400.perlbench  -c $CFG_PATH/$CFG_FILE -o $OUT_PATH -- $BENCH_EXEC/471.omnetpp/471.omnetpp -d omnetpp.ini
