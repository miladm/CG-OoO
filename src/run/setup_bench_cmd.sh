#!/bin/sh
#$ -cwd
#$ -N traceSim
#$ -o output
#$ -e output

ODBP_PATH="/scratch/milad/ondemandbp"
BASE_BENCH_PATH="/home/milad/zsim-apps/inputs"
SPEC_PATH="$ODBP_PATH/spec2006"
OUT_PATH="$ODBP_PATH/milad_benchmark_runs_temp/out"

function setupBenchmark()
{
	benchmark=$1
	if [ $benchmark = perlbench400 ]
	then
		# PERLBENCH #
		bench="$SPEC_PATH/400.perlbench"
		bench_param="-I $BASE_BENCH_PATH/400.perlbench/all/input/lib/ $BASE_BENCH_PATH/400.perlbench/ref/input/checkspam.pl 2500 5 25 11 150 1 1 1 1"
		bench_train_param="-I $BASE_BENCH_PATH/400.perlbench/all/input/lib/ $BASE_BENCH_PATH/400.perlbench/train/input/scrabbl.pl $BASE_BENCH_PATH/400.perlbench/train/input/scrabbl.in"
	elif [ $benchmark = bzip401 ]
	then
		# BZIP2 #
		bench="$SPEC_PATH/401.bzip2"
		bench_param="$BASE_BENCH_PATH/401.bzip2/all/input/input.combined 128"
		bench_train_param="$BASE_BENCH_PATH/401.bzip2/train/input/byoudoin.jpg 5"
	elif [ $benchmark = gcc403 ]
	then
		# GCC #
		bench="$SPEC_PATH/403.gcc"
		bench_param="-C $BASE_BENCH_PATH/403.gcc/ref/input/scilab.i -o out.o"
		bench_train_param="-C $BASE_BENCH_PATH/403.gcc/train/input/integrate.i -o out.o"
	elif [ $benchmark = mcf429 ]
	then
		# MCF #
		bench="$SPEC_PATH/429.mcf"
		bench_param="$BASE_BENCH_PATH/429.mcf/ref/input/inp.in"
		bench_train_param="$BASE_BENCH_PATH/429.mcf/train/input/inp.in"
	elif [ $benchmark = gobmk445 ]
	then
		# GOBMK #
		bench="$SPEC_PATH/445.gobmk"
		bench_param="--quiet --mode gtp < $BASE_BENCH_PATH/445.gobmk/ref/input/13x13.tst"
		bench_train_param="--quiet --mode gtp < $BASE_BENCH_PATH/445.gobmk/train/input/arb.tst"
	elif [ $benchmark = hmmer456 ]
	then
		# HMMER #
		bench="$SPEC_PATH/456.hmmer"
		bench_param="$BASE_BENCH_PATH/456.hmmer/ref/input/nph3.hmm $BASE_BENCH_PATH/456.hmmer/ref/input/swiss41"
		bench_train_param="--fixed 0 --mean 425 --num 85000 --sd 300 --seed 0 $BASE_BENCH_PATH/456.hmmer/train/input/leng100.hmm"
	elif [ $benchmark = sjeng458 ]
	then
		# SJENG #
		bench="$SPEC_PATH/458.sjeng"
		bench_param="$BASE_BENCH_PATH/458.sjeng/ref/input/ref.txt"
		bench_train_param="$BASE_BENCH_PATH/458.sjeng/train/input/train.txt"
	elif [ $benchmark = libquantum462 ]
	then
		# LIBQUANTUM #
		bench="$SPEC_PATH/462.libquantum"
		bench_param="1397 8\""
		bench_train_param="143 25"
	elif [ $benchmark = h264ref464 ]
	then
		# H264REF #
		bench="$SPEC_PATH/464.h264ref"
		bench_param="$BASE_BENCH_PATH/464.h264ref/ref/input/foreman_ref_encoder_baseline.cfg"
		bench_param2="$BASE_BENCH_PATH/464.h264ref/all/input/foreman_qcif.yuv"
		bench_train_param="$BASE_BENCH_PATH/464.h264ref/train/input/foreman_train_encoder_baseline.cfg"
		bench_train_param2="$BASE_BENCH_PATH/464.h264ref/all/input/foreman_qcif.yuv"
	elif [ $benchmark = omnetpp471 ]
	then
		# OMNETPP #
		bench="$SPEC_PATH/471.omnetpp"
		bench_param="-d omnetpp.ini"
		bench_train_param="-d omnetpp.ini"
	elif [ $benchmark = astar473 ]
	then
		# ASTAR #
		bench="$SPEC_PATH/473.astar"
		bench_param="$BASE_BENCH_PATH/473.astar/ref/input/BigLakes2048.cfg"
		bench_train_param="$BASE_BENCH_PATH/473.astar/train/input/BigLakes1024.cfg"
	elif [ $benchmark = xalanc483 ]
	then
		# XALANCBMK #
		bench="$SPEC_PATH/483.xalancbmk"
		bench_param="-v $BASE_BENCH_PATH/483.xalancbmk/ref/input/t5.xml $BASE_BENCH_PATH/483.xalancbmk/ref/input/xalanc.xsl"
		bench_train_param="-v $BASE_BENCH_PATH/483.xalancbmk/train/input/allbooks.xml $BASE_BENCH_PATH/483.xalancbmk/train/input/xalanc.xsl"
	else
		echo "**** Unknown Benchmark Selected; terminating  the run.sh script ****"
		exit 2
	fi
	return
}

