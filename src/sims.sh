./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 5 instructions"
./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 10 instructions"
./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 15 instructions"

./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 35 instructions"
./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 25 instructions"
./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 15 instructions"
./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 10 instructions"
./batch_run.sh "medium pipeline with 4 EU's trying to understand the effect of bbWindow size on performance of BBE - bbSize = 5 instructions"

./batch_run.sh "90 PR's with medium pipeline and 16 bbWindows."

./batch_run.sh "medium pipeline size, 12 bbWindows, 2FU's with adequate number of ports"
./batch_run.sh "medium pipeline size, 12 bbWindows, 8FU's with adequate number of ports"
./batch_run.sh "medium pipeline size, 12 bbWindows, 2FU's without adequate number of ports (unsing ports for a 4-wide machine so the simulator wouldn't get stuck)"
