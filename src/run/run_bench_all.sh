#!/bin/sh
#$ -cwd
#$ -N traceSim
#$ -o output
#$ -e output

ODBP_PATH="/scratch/milad/ondemandbp"
BASE_BENCH_PATH="/home/milad/zsim-apps/inputs"
SPEC_PATH="$ODBP_PATH/spec2006"
OUT_PATH="$ODBP_PATH/milad_benchmark_runs_temp/out"

GEM5_SIM="gem5-nodbp"
MAXINSTS="100000000"
CLOCK="2GHz" #MHz?
L1D_SIZE="32kB"
L1I_SIZE="32kB"
L2_SIZE="1MB"
L3_SIZE="0kB"
L1D_ASSOC="4"
L1I_ASSOC="4"
L2_ASSOC="8"
NUM_DIRS="0"
CACHELINE_SIZE="8"
NUL_L2CACHES="1"
NUL_L3CACHES="0"
NUM_CPUS="1"

CORE_COUNT="16"
THR1=-1.0
THR2=-1.0
MODE='DY'

BP=1
PD=2
FW=2
DISTANCE=`expr $FW \* $PD \- $FW`

LP="_LP"
PAT="_path"

######### BENCHMARKS #########
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

function takeCheckPoint()
{
	b_name=$1
	b_type=$2
	bench_out_base="/scratch/milad/ondemandbp/milad_benchmark_runs_temp/out/$b_name-$b_type"
	if [ ! -d "$bench_out_base" ]; then
		mkdir $bench_out_base
	fi
	for i in ${SP[@]};
	do
		sp=$(($i*$MAXINSTS))
		bench_out="$bench_out_base/$sp-$MAXINSTS"
		if [ ! -d "$bench_out" ]; then
			mkdir $bench_out
		fi
		if [ -d "$bench_out/cpt.None.$sp" ]; then
			echo "NOTE:The checkpoint directory \"$bench_out/cpt.None.$sp\" exists. Skipping this checkpoint job."
			continue
		fi
		echo
		echo "--- ---- --- --- ---- --- --- ---- ---"
		echo "CHECKPOINTING $b_name @ INSTRUCTION $sp"
		echo "--- ---- --- --- ---- --- --- ---- ---"
		######## QSUB#####
            echo "qsub -pe smp 1 -S /bin/sh -N "T$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runT.sh --outdir "$bench_out" --take-checkpoint "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name"" > "$bench_out/chkPt_cmd.sh"
			if [ $b_name = omnetpp471 ]
			then
			   cd "/home/milad/zsim-apps/inputs/471.omnetpp/ref/input/"
               qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "T$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runT.sh --outdir "$bench_out" --take-checkpoint "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name"
			elif [ $b_name = perlbench400 ]
			then
			   cd "/home/milad/zsim-apps/inputs/400.perlbench/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "T$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runT.sh --outdir "$bench_out" --take-checkpoint "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name"
			elif [ $b_name = h264ref464 ]
			then
			   cd "/home/milad/zsim-apps/inputs/464.h264ref/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "T$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runT.sh --outdir "$bench_out" --take-checkpoint "$sp" -c "$bench" -o "$bench_param" -o2 "$bench_param2" --bench-name "$b_name"
			else
               qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "T$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runT.sh --outdir "$bench_out" --take-checkpoint "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name"
			fi
		######## NO QSUB##
			#cd /scratch/milad/ondemandbp/gem5-nodbp
			#echo $PWD && "build/ALPHA_SE/gem5.opt" --outdir="$bench_out"  "configs/example/se.py" --checkpoint-dir=$bench_out --at-instruction --take-checkpoint=$sp --max-checkpoints=1 -c "$bench" -o "$bench_param" && cd -
		##################
	done;
	return
}

function runProfiler()
{
	b_name=$1
	bench_out_base="/scratch/milad/ondemandbp/milad_benchmark_runs_temp/out/$b_name-profile"
	bench_in_base="/scratch/milad/ondemandbp/milad_benchmark_runs_temp/out/$b_name-baseline"
	if [ ! -d "$bench_out_base" ]; then
		echo 
		echo "Expected the checkpoint directory \"$bench_out_base\" to exist"
		exit 2
	fi
	if [ ! -d "$bench_in_base" ]; then
		echo 
		echo "Expected the checkpoint directory \"$bench_in_base\" to exist"
		exit 2
	fi
	for i in ${SP[@]};
	do
		echo $PWD
		sp=$(($i*$MAXINSTS))
		bench_in="$bench_in_base/$sp-$MAXINSTS"
		bench_out="$bench_out_base/$sp-$MAXINSTS"
		if [ ! -d "$bench_out" ]; then
			mkdir $bench_out
		fi
		if [ ! -d "$bench_in" ]; then
			echo "Expected the checkpoint directory \"$bench_in\" to exist"
			exit 2
		fi
		if [ ! -d "$bench_in/cpt.None.$sp" ]; then
			echo "WARNING:The checkpoint directory \"$bench_in/cpt.None.$sp\" does not exist. Skipping the checkpoint."
			continue
		fi
		#if [ -s "$bench_out/nodbp_profile_$b_name.txt" ]; then
		#	echo "NOTE:The checkpoint performance file \"$bench_out/nodbp_profile_$b_name.txt\" exists. Skipping the checkpoint."
		#	continue
		#fi
		echo
		echo "--- ---- --- --- ---- --- --- ---- ---"
		echo "PROFILING $b_namei @ INSTRUCTION $sp"
		echo "--- ---- --- --- ---- --- --- ---- ---"
		######## QSUB #####
			echo "qsub -o "$bench_out" -e "$bench_out" -pe smp 1 -S /bin/sh -N "P$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/run_prof.sh --traceStrt "0" --indir "$bench_in" --outdir "$bench_out" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_train_param" --bench-name "$b_name"" >  "$bench_out/nodbp_profile_cmd.sh"
			if [ $b_name = omnetpp471 ]
			then
			   cd "/home/milad/zsim-apps/inputs/471.omnetpp/train/input/"
			   qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "P$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/run_prof.sh --traceStrt "0" --indir "$bench_in" --outdir "$bench_out" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_train_param" --bench-name "$b_name"
			elif [ $b_name = perlbench400 ]
			then
			   cd "/home/milad/zsim-apps/inputs/400.perlbench/train/input/"
			   qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "P$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/run_prof.sh --traceStrt "0" --indir "$bench_in" --outdir "$bench_out" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_train_param" --bench-name "$b_name"
			elif [ $b_name = h264ref464 ]
			then
			   cd "/home/milad/zsim-apps/inputs/464.h264ref/train/input/"
			   qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "P$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/run_prof.sh --traceStrt "0" --indir "$bench_in" --outdir "$bench_out" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_train_param" -o2 "$bench_train_param2" --bench-name "$b_name"
			else
			   qsub -o "$bench_out" -e "$bench_out" -V -l p=16 -pe smp 1 -S /bin/sh -N "P$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/run_prof.sh --traceStrt "0" --indir "$bench_in" --outdir "$bench_out" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_train_param" --bench-name "$b_name"
			fi
		######## NO QSUB ##
			#/scratch/milad/ondemandbp/gem5-nodbp/build/ALPHA_SE/gem5.opt --outdir=$bench_out --trace-start="0" --stats-file="$bench_out/nodbp_profile_$b_name.txt" --dump-config="$bench_out/config.ini" /scratch/milad/ondemandbp/gem5-nodbp/configs/example/se.py --caches --detailed --checkpoint-dir="$bench_in" --at-instruction --checkpoint-restore="$sp" --maxinsts="$MAXINSTS" -c "$bench" -o "$bench_train_param"
		##################
	done;
	return
}

function runCheckPoint_baseline()
{
	b_name=$1
	b_type=$2
	bench_out_base="/scratch/milad/ondemandbp/milad_benchmark_runs_temp/out/$b_name-$b_type"
	if [ ! -d "$bench_out_base" ]; then
		echo 
		echo "Expected the checkpoint directory \"$bench_out_base\" to exist"
		exit 2
	fi
	for i in ${SP[@]};
	do
		$chert
		echo $PWD
		sp=$(($i*$MAXINSTS))
		bench_out="$bench_out_base/$sp-$MAXINSTS"
		if [ ! -d "$bench_out" ]; then
			echo "Expected the checkpoint directory \"$bench_out\" to exist"
			exit 2
		fi
		if [ ! -d "$bench_out/cpt.None.$sp" ]; then
			echo "WARNING:The checkpoint directory \"$bench_out/cpt.None.$sp\" does not exist. Skipping the checkpoint."
			continue
		fi
		#if [ -s "$bench_out/nodbp_runChkPtStat_$b_name.txt" ]; then
		#	echo "NOTE:The checkpoint performance file \"$bench_out/nodbp_runChkPtStat_$b_name.txt\" exists. Skipping the checkpoint."
		#	continue
		#fi
		echo
		echo "--- ---- --- --- ---- --- --- ---- ---"
		echo "RUNNING $b_name @ INSTRUCTION $sp"
		echo "--- ---- --- --- ---- --- --- ---- ---"
		######## QSUB #####
            echo "qsub -o "$bench_out" -e "$bench_out" -p -10 -pe smp 1 -S /bin/sh -N "N$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_nodbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name"  --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"" > "$bench_out/nodbp_runChkPt_cmd.sh"
			if [ $b_name = omnetpp471 ]
			then
			   cd "/home/milad/zsim-apps/inputs/471.omnetpp/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -p -10 -pe smp 1 -S /bin/sh -N "N$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_nodbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			elif [ $b_name = perlbench400 ]
			then
			   cd "/home/milad/zsim-apps/inputs/400.perlbench/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -p -10 -pe smp 1 -S /bin/sh -N "N$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_nodbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			elif [ $b_name = h264ref464 ]
			then
				cd "/home/milad/zsim-apps/inputs/464.h264ref/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -p -10 -pe smp 1 -S /bin/sh -N "N$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_nodbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_param" -o2 "$bench_param2" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			else
			   qsub -o "$bench_out" -e "$bench_out" -p -10 -pe smp 1 -S /bin/sh -N "N$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_nodbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" -c "$bench" -o "$bench_param" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			fi
		######## NO QSUB ##
			#cd /scratch/milad/ondemandbp/gem5-nodbp
			#echo $PWD && "build/ALPHA_SE/gem5.opt" --outdir="$bench_out" --stats-file="$bench_out/runChkPtStat_$b_name.txt"  "configs/example/se.py" --num-l2caches=$NUL_L2CACHES --num-l3caches=$NUL_L3CACHES --l1d_size=$L1D_SIZE --l1i_size=$L1I_SIZE --l2_size=$L2_SIZE --l3_size=$L3_SIZE --l1d_assoc=$L1D_ASSOC --l1i_assoc=$L1I_ASSOC --l2_assoc=$L2_ASSOC --caches --l2cache --detailed --checkpoint-dir=$bench_out --at-instruction --checkpoint-restore=$sp --maxinsts=$MAXINSTS -c "$bench" -o "$bench_param"  > "$bench_out/runChkPt_cmd.sh" && cd -
		##################
	done;
	#cd -
	return
}

function runCheckPoint_odbp()
{
	b_name=$1
	b_type=$2
	bench_out_base="/scratch/milad/ondemandbp/milad_benchmark_runs_temp/out/$b_name-$b_type"
	if [ ! -d "$bench_out_base" ]; then
		echo "Expected the checkpoint directory \"$bench_out_base\" to exist"
		exit 2
	fi
	for i in ${SP[@]};
	do
		$chert
		echo $PWD
		sp=$(($i*$MAXINSTS))
		bench_out="$bench_out_base/$sp-$MAXINSTS"
		if [ ! -d "$bench_out" ]; then
			echo "Expected the checkpoint directory \"$bench_out\" to exist"
			exit 2
		fi
		if [ ! -d "$bench_out/cpt.None.$sp" ]; then
			echo "WARNING:The checkpoint directory \"$bench_out/cpt.None.$sp\" does not exist. Skipping the checkpoint."
			continue
		fi
		#if [ -s "$bench_out/odbp_runChkPtStat_$b_name-$THR1-$THR2-$MODE.txt" ]; then
		#	echo "NOTE:The checkpoint performance file \"$bench_out/odbp_runChkPtStat_$b_name-$THR1-$THR2-$MODE.txt\" exists. Skipping the checkpoint."
		#	continue
		#fi
		if [ ! -s "$hint_file_thr" ]; then
			echo "NOTE:The policy file \"$hint_file_thr\" is unavailable . Skipping the run."
			continue
		fi
		echo
		echo "--- ---- --- --- ---- --- --- ---- ---"
		echo "RUNNING $b_name @ INSTRUCTION $sp"
		echo "--- ---- --- --- ---- --- --- ---- ---"
		######## QSUB #####
			#TODO: you should add the link for h264ref benchmark and also update runF_odbp accordingly
            echo "qsub -o "$bench_out" -e "$bench_out" -pe smp 1 -S /bin/sh -N "O$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_odbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" --thr1 "$THR1" --thr2 "$THR2" --mode "$MODE" -c "$bench" -o "$bench_param" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"" > "$bench_out/odbp_runChkPt_cmd-$THR1-$THR2-$MODE.sh"
			if [ $b_name = omnetpp471 ]
			then
			   cd "/home/milad/zsim-apps/inputs/471.omnetpp/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -pe smp 1 -S /bin/sh -N "O$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_odbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" --thr1 "$THR1" --thr2 "$THR2" --mode "$MODE" -c "$bench" -o "$bench_param" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			elif [ $b_name = perlbench400 ]
			then
			   cd "/home/milad/zsim-apps/inputs/400.perlbench/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -pe smp 1 -S /bin/sh -N "O$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_odbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" --thr1 "$THR1" --thr2 "$THR2" --mode "$MODE" -c "$bench" -o "$bench_param" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			elif [ $b_name = h264ref464 ]
			then
			   cd "/home/milad/zsim-apps/inputs/464.h264ref/ref/input/"
			   qsub -o "$bench_out" -e "$bench_out" -pe smp 1 -S /bin/sh -N "O$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_odbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" --thr1 "$THR1" --thr2 "$THR2" --mode "$MODE" -c "$bench" -o "$bench_param" -o2 "$bench_param2" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			else
			   qsub -o "$bench_out" -e "$bench_out" -pe smp 1 -S /bin/sh -N "O$i-$b_name" /scratch/milad/ondemandbp/milad_benchmark_runs_temp/runF_odbp.sh --outdir "$bench_out" --num-l2caches "$NUL_L2CACHES" --num-l3caches "$NUL_L3CACHES" --l1d_size "$L1D_SIZE" --l1i_size "$L1I_SIZE" --l2_size "$L2_SIZE" --l1d_assoc "$L1D_ASSOC" --l1i_assoc "$L1I_ASSOC" --l2_assoc "$L2_ASSOC" --maxinsts "$MAXINSTS" --checkpoint-restore "$sp" --thr1 "$THR1" --thr2 "$THR2" --mode "$MODE" -c "$bench" -o "$bench_param" --bench-name "$b_name" --hint_file "$hint_file_thr" --bp "$BP" --pd "$PD" --fw "$FW"
			fi
		######## NO QSUB ##
			#cd /scratch/milad/ondemandbp/gem5
			#echo $PWD && "build/ALPHA_SE/gem5.opt" --outdir="$bench_out" --stats-file="$bench_out/runChkPtStat_$b_name.txt"  "configs/example/se.py" --num-l2caches=$NUL_L2CACHES --num-l3caches=$NUL_L3CACHES --l1d_size=$L1D_SIZE --l1i_size=$L1I_SIZE --l2_size=$L2_SIZE --l3_size=$L3_SIZE --l1d_assoc=$L1D_ASSOC --l1i_assoc=$L1I_ASSOC --l2_assoc=$L2_ASSOC --caches --l2cache --detailed --checkpoint-dir=$bench_out --at-instruction --checkpoint-restore=$sp --maxinsts=$MAXINSTS --hint_file="$hint_file" -c "$bench" -o "$bench_param"  > "$bench_out/runChkPt_cmd.sh" && cd -
			#ECHo ""build/ALPHA_SE/gem5.opt" --outdir="$bench_out" --stats-file="$bench_out/runChkPtStat_$b_name.txt"  "configs/example/se.py" --num-l2caches=$NUL_L2CACHES --num-l3caches=$NUL_L3CACHES --l1d_size=$L1D_SIZE --l1i_size=$L1I_SIZE --l2_size=$L2_SIZE --l3_size=$L3_SIZE --l1d_assoc=$L1D_ASSOC --l1i_assoc=$L1I_ASSOC --l2_assoc=$L2_ASSOC --caches --l2cache --detailed --checkpoint-dir=$bench_out --at-instruction --checkpoint-restore=$sp --maxinsts=$MAXINSTS --hint_file="$hint_file" -c "$bench" -o "$bench_param"  > "$bench_out/runChkPt_cmd.sh""
		##################
	done;
	#cd -
	return
}

function parseSP()
{
	filename=$1
	echo "--------------------"
	echo OBTAIN SIMPOINT SPs
	echo "--------------------"
	#read fname
	exec<$filename
	value=0
	line_num=0
	while read line
	do
		line_num=$line_num+1
		IFS=' ' read -ra val <<< "$line"
		echo $val
		SP[$line_num]=$val
	done
	return
}

function parseWeight()
{
	filename=$1
	echo "-------------------------"
	echo OBTAIN SIMPOINT WEIGHTS
	echo "-------------------------"
	#read fname
	exec<$filename
	value=0
	line_num=0
	while read line
	do
		line_num=$line_num+1
		IFS=' ' read -ra val <<< "$line"
		echo $val
		WT[$line_num]=$val
	done
	return
}

# MAIN
bench_name="bzip401"
if [ "$#" -ne 9 ]; then
	echo
	echo 'USAGE: ./run.sh <benchmarkname> <task>'
	echo ---------------------------
	echo Choose Benchmark Name From the List:
	echo - perlbench400
	echo - bzip401
	echo - gcc403 - CURRENTLY NOT SUPPORTED
	echo - mcf429
	echo - gobmk445
	echo - hmmer456
	echo - sjeng458
	echo - libquantum462
	echo - h264ref464
	echo - omnetpp471
	echo - astar473
	echo - xalanc483
	echo ---------------------------
	echo Choose Your Task from the List - T, N or O:
	echo T - Take Checkpoint
	echo N - Fast Forward to Checkpoint - BASELINE or NODBP
	echo O - Fast Forward to Checkpoint - ODBP
	echo ---------------------------
	exit 1
fi

bench_name=$1
task=$2
CORE_COUNT=$3
BP=$4
PD=$5
FW=$6
THR1=$7
THR2=$8
MODE=$9
DISTANCE=`expr $FW \* $PD \- $FW`
echo "distance:  $DISTANCE"

setupBenchmark "$bench_name"
parseSP "./simpoints/simpoints_$bench_name"
parseWeight "./simpoints/weights_$bench_name"

if [ $task = T ]
then
	echo "----------------------"
	echo ">>> TAKING CHECKPOINTS"
	echo "----------------------"
	takeCheckPoint $bench_name baseline
elif [ $task = N ]
then
	echo "----------------------------------"
	echo ">>> RUNNING CHECKPOINTS - BASELINE"
	echo "----------------------------------"
	runCheckPoint_baseline $bench_name baseline
elif [ $task = O ]
then
	echo "------------------------------"
	echo ">>> RUNNING CHECKPOINTS - ODBP"
	echo "------------------------------"
	runCheckPoint_odbp $bench_name baseline
elif [ $task = P ]
then
	echo "------------------------------"
	echo ">>> RUNNING PROFILING         "
	echo "------------------------------"
	runProfiler $bench_name
else
	echo "**** Unknown Task Selected; terminating  the run.sh script ****"
	exit 2
fi
