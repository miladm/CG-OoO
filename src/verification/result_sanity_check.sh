RESULT_PATH='/scratch/milad/qsub_outputs/perf_sim_test/out1'

cd $RESULT_PATH
echo "============================="
echo "CHCECK INSTRUCTION COUNTS"
grep "commit.ins_cnt" 4*

echo "============================="
echo "CHCECK INSTRUCTION COUNTS"
grep "CHECK FILE TIMESTAMPS"
llt 4*
