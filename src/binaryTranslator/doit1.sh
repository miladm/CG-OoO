#!/bin/sh

REG_ALLOC_MODE="local_global_reg"
SCH_MODE="no_list_sch"
CLUSTER_MODE="bb"
CLUSTER_SIZE=50

# ./PhraseFormer 400.perlbench $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 445.gobmk $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 464.h264ref $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 473.astar $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 471.omnetpp $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
 ./PhraseFormer 483.xalancbmk $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 462.libquantum  $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 458.sjeng $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 456.hmmer $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 429.mcf $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 401.bzip2 $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE
# ./PhraseFormer 403.gcc $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE

# ./PhraseFormer test $REG_ALLOC_MODE $SCH_MODE $CLUSTER_MODE $CLUSTER_SIZE

#####
#./parse_x86.py cfg input_files/ $1 > input_files/input_assembly.s
#./PhraseFormer
#cd dotFiles
#make
#cd ../
