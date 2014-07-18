#!/bin/sh

PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
OUT_PATH="/scratch/milad/qsub_outputs/simpoint_exe_test/out1"
RUN_PATH="$PARS_ROOT/run"
SPEC_ROOT="/home/milad/zsim-apps"
FILE=$OUT_PATH/README

###########################################
# Adding README file
###########################################
 NOW=$(date +"%Y_%m_%d")
 if [ -f $FILE ];
 then
	echo "File \"$FILE\" exists."
 else
	echo "File \"$FILE\" does not exist. Creating it..."
	touch $FILE
	echo "Milad Mohammadi" > $FILE
	echo $NOW >> $FILE
	echo "---------------" >> $FILE
	echo $@ >> $FILE
 fi
###########################################


#$PIN_ROOT/pin -t $PARS_ROOT/obj-intel64/main_pars.so -- $SPEC_ROOT/build/speccpu2006/462.libquantum/462.libquantum 1397 8
#echo "$PIN_ROOT/pin -t $PARS_ROOT/obj-intel64/main_pars.so -- $SPEC_ROOT/build/speccpu2006/462.libquantum/462.libquantum 1397 8"

## QSUB
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_gcc        $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/403.gcc/403.gcc -C $SPEC_ROOT/inputs/403.gcc/ref/input/scilab.i -o $OUT_PATH/gcc_out.o
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_mcf        $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/429.mcf/429.mcf $SPEC_ROOT/inputs/429.mcf/ref/input/inp.in
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_bzip2      $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/401.bzip2/401.bzip2 $SPEC_ROOT/inputs/401.bzip2/ref/input/input.source 64
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_hmmer      $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/456.hmmer/456.hmmer $SPEC_ROOT/inputs/456.hmmer/ref/input/nph3.hmm $SPEC_ROOT/inputs/456.hmmer/ref/input/swiss41
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_sjeng      $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/458.sjeng/458.sjeng $SPEC_ROOT/inputs/458.sjeng/ref/input/ref.txt
 qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_gobmk      $RUN_PATH/run.sh "$SPEC_ROOT/build/speccpu2006/445.gobmk/445.gobmk --quiet --mode gtp < $SPEC_ROOT/inputs/445.gobmk/ref/input/13x13.tst"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_libquantum $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/462.libquantum/462.libquantum 1397 8
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_h264ref    $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/464.h264ref/464.h264ref  -d $SPEC_ROOT/inputs/464.h264ref/ref/input/foreman_ref_encoder_baseline.cfg -p InputFile="$SPEC_ROOT/inputs/464.h264ref/all/input/foreman_qcif.yuv"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_astar      $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/473.astar/473.astar $SPEC_ROOT/inputs/473.astar/ref/input/BigLakes2048.cfg
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_xalancbmk  $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/483.xalancbmk/483.xalancbmk -v $SPEC_ROOT/inputs/483.xalancbmk/ref/input/t5.xml $SPEC_ROOT/inputs/483.xalancbmk/ref/input/xalanc.xsl
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_perlbench  $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/400.perlbench/400.perlbench -I $SPEC_ROOT/inputs/400.perlbench/all/input/lib/ $SPEC_ROOT/inputs/400.perlbench/ref/input/checkspam.pl 2500 5 25 11 150 1 1 1 1

#cd "$SPEC_ROOT/inputs/471.omnetpp/ref/input/"
#echo "$SPEC_ROOT/inputs/471.omnetpp/ref/input/"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N re_omnetpp    $RUN_PATH/run.sh $SPEC_ROOT/build/speccpu2006/471.omnetpp/471.omnetpp -d omnetpp.ini
