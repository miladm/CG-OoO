RESULT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/out1'

echo "============================="
echo "CHCECK INSTRUCTION COUNTS"
cd $RESULT_PATH
grep "commit.ins_cnt" 4*

echo "============================="
echo "CHCECK INSTRUCTION COUNTS"
cd $RESULT_PATH
grep "CHECK FILE TIMESTAMPS"
llt 4*
