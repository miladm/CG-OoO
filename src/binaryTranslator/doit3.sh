#!/bin/sh

REG_ALLOC_MODE="global_reg"
SCH_MODE="list_sch"

# ./PhraseFormer 400.perlbench $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 445.gobmk $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 464.h264ref $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 473.astar $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 471.omnetpp $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 483.xalancbmk $REG_ALLOC_MODE $SCH_MODE
 ./PhraseFormer 462.libquantum  $REG_ALLOC_MODE $SCH_MODE
 ./PhraseFormer 458.sjeng $REG_ALLOC_MODE $SCH_MODE
 ./PhraseFormer 456.hmmer $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 429.mcf $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 403.gcc $REG_ALLOC_MODE $SCH_MODE
# ./PhraseFormer 401.bzip2 $REG_ALLOC_MODE $SCH_MODE



#####
#./parse_x86.py cfg input_files/ $1 > input_files/input_assembly.s
#./PhraseFormer
#cd dotFiles
#make
#cd ../
