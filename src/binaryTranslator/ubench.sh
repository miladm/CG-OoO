#!/bin/sh

REG_ALLOC_MODE="local_global_reg"
SCH_MODE="no_list_sch"

UBENCH_NAME="ub_simple_cfg_loop"
./PhraseFormer $UBENCH_NAME $REG_ALLOC_MODE $SCH_MODE
