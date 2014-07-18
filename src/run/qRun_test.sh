#!/bin/sh

PARS_ROOT='/home/milad/esc_project/svn/PARS/src'
PIN_ROOT='/home/milad/esc_project/svn/pin-2.12'
OUT_PATH="/scratch/milad/qsub_outputs/wront_path/out2"

##@ TERMINAL
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/403.gcc/403.gcc -C /home/milad/zsim-apps/inputs/403.gcc/test/input/cccp.i -o out.o
#../../../pin -separate_memory -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/esc_project/svn/benchmarks/spec2006/mcf-1.3_2/src/mcf /home/milad/esc_project/svn/benchmarks/spec2006/mcf-1.3_2/src/inp_small.in
#../../../pin -pin_memory_range 0x80000000:0x90000000 -pause_tool 15 -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/bzip2  -kf /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/sample2.ref
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/bzip2  -kf /home/milad/esc_project/svn/benchmarks/spec2006/bzip2/sample2.ref
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/456.hmmer/456.hmmer --fixed 0 --mean 325 --num 45000 --sd 200 --seed 0  /home/milad/zsim-apps/inputs/456.hmmer/test/input/bombesin.hmm
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/458.sjeng/458.sjeng /home/milad/zsim-apps/inputs/458.sjeng/test/input/test.txt
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/445.gobmk/445.gobmk --quiet --mode gtp < /home/milad/zsim-apps/inputs/445.gobmk/test/input/capture.tst
$PIN_ROOT/pin -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/462.libquantum/462.libquantum 33 5
echo "$PIN_ROOT/pin -t $PARS_ROOT/obj-intel64/main_pars.so -- /home/milad/zsim-apps/build/speccpu2006/462.libquantum/462.libquantum 33 5"
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/464.h264ref/464.h264ref  -d /home/milad/zsim-apps/inputs/464.h264ref/test/input/foreman_test_encoder_baseline.cfg -p InputFile="/home/milad/zsim-apps/inputs/464.h264ref/all/input/foreman_qcif.yuv"
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/471.omnetpp/471.omnetpp -d /home/milad/zsim-apps/inputs/471.omnetpp/test/input/omnetpp.ini
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/473.astar/473.astar /home/milad/zsim-apps/inputs/473.astar/test/input/lake.cfg
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/483.xalancbmk/483.xalancbmk -v /home/milad/zsim-apps/inputs/483.xalancbmk/test/input/test.xml /home/milad/zsim-apps/inputs/483.xalancbmk/test/input/xalanc.xsl
#../../../pin -t obj-intel64/wrong_path.so  -usectxt -- /home/milad/zsim-apps/build/speccpu2006/400.perlbench/400.perlbench -I /home/milad/zsim-apps/inputs/400.perlbench/ref/lib/test.xml /home/milad/zsim-apps/inputs/400.perlbench/test/input/attrs.pl


## QSUB
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_gcc run.sh /home/milad/zsim-apps/build/speccpu2006/403.gcc/403.gcc -C /home/milad/zsim-apps/inputs/403.gcc/test/input/cccp.i -o $OUT_PATH/out.o
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_mcf run.sh /home/milad/zsim-apps/build/speccpu2006/429.mcf/429.mcf /home/milad/zsim-apps/inputs/429.mcf/test/input/inp.in
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_bzip2 run.sh /home/milad/zsim-apps/build/speccpu2006/401.bzip2/401.bzip2 /home/milad/zsim-apps/inputs/401.bzip2/test/input/dryer.jpg 2
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_hmmer run.sh /home/milad/zsim-apps/build/speccpu2006/456.hmmer/456.hmmer --fixed 0 --mean 325 --num 45000 --sd 200 --seed 0  /home/milad/zsim-apps/inputs/456.hmmer/test/input/bombesin.hmm
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_sjeng run.sh /home/milad/zsim-apps/build/speccpu2006/458.sjeng/458.sjeng /home/milad/zsim-apps/inputs/458.sjeng/test/input/test.txt
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_gobmk run.sh /home/milad/zsim-apps/build/speccpu2006/445.gobmk/445.gobmk --quiet --mode gtp < /home/milad/zsim-apps/inputs/445.gobmk/test/input/capture.tst
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_libquantum run.sh /home/milad/zsim-apps/build/speccpu2006/462.libquantum/462.libquantum 33 5
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_h264ref run.sh /home/milad/zsim-apps/build/speccpu2006/464.h264ref/464.h264ref  -d /home/milad/zsim-apps/inputs/464.h264ref/test/input/foreman_test_encoder_baseline.cfg -p InputFile="/home/milad/zsim-apps/inputs/464.h264ref/all/input/foreman_qcif.yuv"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_astar run.sh /home/milad/zsim-apps/build/speccpu2006/473.astar/473.astar /home/milad/zsim-apps/inputs/473.astar/test/input/lake.cfg
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_xalancbmk run.sh /home/milad/zsim-apps/build/speccpu2006/483.xalancbmk/483.xalancbmk -v /home/milad/zsim-apps/inputs/483.xalancbmk/test/input/test.xml /home/milad/zsim-apps/inputs/483.xalancbmk/test/input/xalanc.xsl
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_perlbench run.sh /home/milad/zsim-apps/build/speccpu2006/400.perlbench/400.perlbench -I. -I /home/milad/zsim-apps/inputs/400.perlbench/all/input/lib/ /home/milad/zsim-apps/inputs/400.perlbench/test/input/attrs.pl
#cd "/home/milad/zsim-apps/inputs/471.omnetpp/test/input/"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N ts_omnetpp /home/milad/pin-2.12-58423-gcc.4.4.7-linux/source/tools/wrong_path/run.sh /home/milad/zsim-apps/build/speccpu2006/471.omnetpp/471.omnetpp -d omnetpp.ini
