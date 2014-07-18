## QSUB
#!/bin/sh

OUT_PATH="/scratch/milad/qsub_outputs/wront_path/out1"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_gcc run.sh /home/milad/zsim-apps/build/speccpu2006/403.gcc/403.gcc -C /home/milad/zsim-apps/inputs/403.gcc/train/input/integrate.i -o $OUT_PATH/out.o
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_mcf run.sh /home/milad/zsim-apps/build/speccpu2006/429.mcf/429.mcf /home/milad/zsim-apps/inputs/429.mcf/train/input/inp.in
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_bzip2 run.sh /home/milad/zsim-apps/build/speccpu2006/401.bzip2/401.bzip2 /home/milad/zsim-apps/inputs/401.bzip2/train/input/byoudoin.jpg 5
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_hmmer run.sh /home/milad/zsim-apps/build/speccpu2006/456.hmmer/456.hmmer --fixed 0 --mean 425 --num 85000 --sd 300 --seed 0  /home/milad/zsim-apps/inputs/456.hmmer/train/input/leng100.hmm
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_sjeng run.sh /home/milad/zsim-apps/build/speccpu2006/458.sjeng/458.sjeng /home/milad/zsim-apps/inputs/458.sjeng/train/input/train.txt
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_gobmk run.sh /home/milad/zsim-apps/build/speccpu2006/445.gobmk/445.gobmk --quiet --mode gtp < /home/milad/zsim-apps/inputs/445.gobmk/train/input/arb.tst
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_libquantum run.sh /home/milad/zsim-apps/build/speccpu2006/462.libquantum/462.libquantum 143 25
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_h264ref run.sh /home/milad/zsim-apps/build/speccpu2006/464.h264ref/464.h264ref  -d /home/milad/zsim-apps/inputs/464.h264ref/train/input/foreman_train_encoder_baseline.cfg -p InputFile="/home/milad/zsim-apps/inputs/464.h264ref/all/input/foreman_qcif.yuv"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_astar run.sh /home/milad/zsim-apps/build/speccpu2006/473.astar/473.astar /home/milad/zsim-apps/inputs/473.astar/train/input/BigLakes1024.cfg
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_xalancbmk run.sh /home/milad/zsim-apps/build/speccpu2006/483.xalancbmk/483.xalancbmk -v /home/milad/zsim-apps/inputs/483.xalancbmk/train/input/allbooks.xml /home/milad/zsim-apps/inputs/483.xalancbmk/train/input/xalanc.xsl
 cd "/home/milad/zsim-apps/inputs/400.perlbench/train/input/"
 qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_perlbench /home/milad/pin-2.12-58423-gcc.4.4.7-linux/source/tools/wrong_path/run.sh /home/milad/zsim-apps/build/speccpu2006/400.perlbench/400.perlbench -I /home/milad/zsim-apps/inputs/400.perlbench/all/input/lib/ /home/milad/zsim-apps/inputs/400.perlbench/train/input/scrabbl.pl /home/milad/zsim-apps/inputs/400.perlbench/train/input/scrabbl.in
#cd "/home/milad/zsim-apps/inputs/471.omnetpp/train/input/"
#qsub -e $OUT_PATH -o $OUT_PATH -V -l p=16 -pe smp 2 -S /bin/sh -N tr_omnetpp /home/milad/pin-2.12-58423-gcc.4.4.7-linux/source/tools/wrong_path/run.sh /home/milad/zsim-apps/build/speccpu2006/471.omnetpp/471.omnetpp -d omnetpp.ini
