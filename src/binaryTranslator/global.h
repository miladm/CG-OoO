/*******************************************************************************
 * global.h define global parameters
 ******************************************************************************/
#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <map>

typedef long long unsigned int ADDR;
typedef enum {none, READ, WRITE} memType;
typedef enum {noType, ALU, MEM, FPU, BR} type;
typedef enum {LRF, GRF} regKind;
typedef enum {GLOBAL, LOCAL_GLOBAL} REG_ALLOC_MODE;
typedef enum {NO_LIST_SCH, LIST_SCH} SCH_MODE;
typedef enum {STORE_ORDER, LOAD_STORE_ORDER} MEM_SCH_MODE;

#define CFG_STRING_SIZE 400
#define INS_STRING_SIZE 330
#define REG_STRING_SIZE 20
#define OPCODE_STRING_SIZE 30
//Found using: awk ' { if ( length > x ) { x = length } }END{ print x }' x86_opcode.txt

//PROFILING
#define UPLD_THRESHOLD 0.01
#define WBB_LOWER_BOUND 0.1
#define WBB_UPPER_BOUND 0.9

//DEBUG FLAGS
#define DEBUG_MAIN 0
#define DEBUG_INS 0
#define DEBUG_BB 0
#define DEBUG_PB 0
#define DEBUG_RA 0
// #define DEBUG_DOM 0
// #define DEBUG_SSA 0

//DATA DEPENDENCY
#define NUM_REGISTERS 68
#define INIT_RENAME_REG_NUM 100 //TODO change this number - not 

//LATENCIES
#define ALU_LATENCY 1
#define FPU_LATENCY 5
#define BR_LATENCY 3 //IF 1 cycles + ID 2 cycles (should be 5 for OOO?)
#define L1_LATENCY 4
#define L2_LATENCY 20
#define L3_LATENCY 40
#define ST_LATENCY 2 //To make it more high priority then ALU ops
#define MEM_LATENCY 100
#define MEM_HIGHERARCHY 4

//HARDWARE SPECIFICATIONS
#define NUM_EU 1

//RESGITER ALLOCATION
#define LRF_SIZE 100
#define GRF_SIZE 60

#define LRF_LO 100
#define LRF_HI LRF_LO + LRF_SIZE - 1
#define GRF_LO LRF_HI + 1
#define GRF_HI GRF_LO + GRF_SIZE - 1

#define X86_REG_LO 1
#define X86_REG_HI 68

#define INVALID_REG -1

//PHI FUNCTION
#define PHI_INS_ADDR 1

#endif
