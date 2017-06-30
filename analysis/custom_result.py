#!/usr/bin/env python
from __future__ import division
from collections import deque
from collections import defaultdict
from time import time
import os, glob, os.path
import sys
import re
import os

# VERIFY THE NUMBER OF EXE INSTRUCTIONS IS SUFFICIENTLY LARGE
def stages (result_map):
    bp_energy (result_map)
    fetch_energy (result_map)
    decode_rr_energy (result_map)
    issue_energy (result_map)
    execute_energy (result_map)
    memory_energy (result_map)
    rf_energy (result_map)
#    l2_l3_energy (result_map)
    commit_energy (result_map)

def get_eu_leakage (result_map):
    num_eu = 4#12
    num_fpu = 4#12
    clk_cycles  = result_map['sysClock.clk_cycles']
    int_eu_leakage_per_cycle_coef = 0.01
    fp_eu_leakage_per_cycle_coef = 0.04
    int_eu_energy_per_cycle = 1.040 * 2 #Address gen + ALU
    fp_eu_energy_per_cycle = 8.800
    int_eu_leakage_per_cycle = num_eu * int_eu_leakage_per_cycle_coef * int_eu_energy_per_cycle
    fp_eu_leakage_per_cycle = num_fpu * fp_eu_leakage_per_cycle_coef * fp_eu_energy_per_cycle
    eu_leakage_per_cycle = int_eu_leakage_per_cycle + fp_eu_leakage_per_cycle
    eu_leakage = eu_leakage_per_cycle * clk_cycles
    return eu_leakage

def postProcessEnergies(result_map):
    result_map['TOTAL.Energy'] += get_eu_leakage (result_map)
    result_map['STATIC.Energy'] += get_eu_leakage (result_map)

def bp_energy (result_map):
    energy = 0

    energy += result_map['2bc_gskew.e_ram']
    energy += result_map['2bc_gskew.e_leak']
    energy += result_map['2bc_gskew.wire.e_w_bp2cache_o3']
    energy += result_map['2bc_gskew.wire.e_w_bp2cache_ino']
    energy += result_map['2bc_gskew.wire.e_w_bp2cache_bb']
    energy += result_map['BTB.e_cam']
    energy += result_map['BTB.e_leak']
    energy += result_map['branchPred.e_ff']
    energy += result_map['branchPred.e_leak']

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['bpu.epo'] = epo
    result_map['bpu.power'] = energy
    result_map['bpu'] = energy

def fetch_energy (result_map):
    energy = 0

    energy += result_map['fetch.e_ff']
    energy += result_map['fetch.e_leak']
    energy += result_map['l1_i_0.e_ram']
    energy += result_map['l1_i_0.e_leak']
    energy += result_map['itlb.e_cam']

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['fetch.epo'] = epo
    result_map['fetch.power'] = energy
    result_map['fetch'] = energy

def decode_rr_energy (result_map):
    energy = 0

    energy += result_map['GlobalRegisterRename.rd_wire.e_wire']
    energy += result_map['registerRename.rd_wire.e_wire']
    energy += result_map['registerFile.rr.wire.e_w_cache2rr']
    energy += result_map['grfManager.rr.wire.e_w_cache2rr']
    energy += result_map['grfManager.grat.e_ram']
    energy += result_map['grfManager.gapr.e_ram']
    energy += result_map['grfManager.garst.e_ram']
    energy += result_map['rfManager.rat.e_ram']
    energy += result_map['rfManager.apr.e_ram']
    energy += result_map['rfManager.arst.e_ram']
    energy += result_map['decode.e_ff']
    energy += result_map['decode.e_leak']

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['decode-rename.epo'] = epo
    result_map['decode-rename.power'] = energy
    result_map['decode-rename'] = energy

def rename_energy (result_map):
    energy = 0

    energy += result_map['grfManager.rr.wire.e_w_cache2rr']
    energy += result_map['grfManager.grat.e_ram']
    energy += result_map['grfManager.gapr.e_ram']
    energy += result_map['grfManager.garst.e_ram']

    energy += result_map['rfManager.rat.e_ram']
    energy += result_map['rfManager.apr.e_ram']
    energy += result_map['rfManager.arst.e_ram']

    all_ops = result_map['commit.ins_cnt']
    epo = energy / float(all_ops)

    result_map['rename.Energy'] = energy
    result_map['rename.epo'] = epo

def issue_energy (result_map):
    energy = 0

    energy += issue_energy_cam (result_map)
    energy += issue_energy_ram (result_map)
    energy += issue_energy_rest (result_map)

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['issue.epo'] = epo
    result_map['issue.power'] = energy
    result_map['issue'] = energy

def issue_energy_cam (result_map):
    energy = 0.0

    energy += result_map['bbWinBuf.e_cam']
    energy += result_map['bbWindow.e_cam']
    energy += result_map['ResStn_0.e_cam']
    energy += result_map['ResStn_0.e_cam2']

    all_ops     = result_map['commit.ins_cnt']
    epo         = energy / float(all_ops)

    result_map['issue_cam.epo'] = epo
    result_map['issue_cam'] = energy

    return energy
    
def issue_energy_ram (result_map):
    energy = 0.0

    energy += result_map['iWindow.e_ram']
    energy += result_map['ResStn_0.e_ram']
    energy += result_map['bbWindow.e_ram']
    energy += result_map['bbWinBuf.e_ram']

    all_ops     = result_map['commit.ins_cnt']
    epo         = energy / float(all_ops)

    result_map['issue_ram.epo'] = epo
    result_map['issue_ram'] = energy

    return energy
    
def issue_energy_rest (result_map):
    energy = 0.0

    energy += result_map['iWindow.wr_wire.e_w_cache2win']
    energy += result_map['schedule.e_ff']
    energy += result_map['schedule.e_leak']
    energy += result_map['iWindow.e_leak']
    energy += result_map['iWindow.rd_wire.e_wire']
    energy += result_map['iWindow.wr_wire.e_w_win2eu']
    energy += result_map['ResStn_0.wr_wire.e_w_rr2rs']
    energy += result_map['ResStn_0.rd_wire.e_w_rs2eu']
    energy += result_map['ResStn_0.e_leak']
    energy += result_map['bbWindow.wr_wire.e_w_rr2iq']
    energy += result_map['bbWindow.rd_wire.e_w_iq2eu']
    energy += result_map['bbWindow.e_leak']
    energy += result_map['bbWinBuf.e_leak']

    all_ops     = result_map['commit.ins_cnt']
    epo         = energy / float(all_ops)

    result_map['issue_rest.epo'] = epo
    result_map['issue_rest'] = energy

    return energy

def execute_energy (result_map):
    energy = 0

    energy += result_map['EU.e_eu']
    energy += result_map['execution.e_ff']
    energy += result_map['execution.e_leak']
    energy += get_eu_leakage (result_map)

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']

    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['execute.epo'] = epo
    result_map['execute.power'] = power
    result_map['execute'] = energy

def memory_energy (result_map):
    energy = 0

    energy += result_map['stBuff.e_ram']
    energy += result_map['stBuff.e_leak']
    energy += result_map['SQ.wr_wire.e_w_eu2lsq_bb']
    energy += result_map['SQ.rd_wire.e_w_rr2lsq_bb']
    energy += result_map['SQ.rd_wire.e_w_cache2lsq_bb']
    energy += result_map['SQ.rd_wire.e_w_lsq2brob']
    energy += result_map['SQ.wr_wire.e_w_eu2lsq_o3']
    energy += result_map['SQ.rd_wire.e_w_rr2lsq_o3']
    energy += result_map['SQ.rd_wire.e_w_cache2lsq_o3']
    energy += result_map['SQ.e_cam']
    energy += result_map['SQ.e_ram']
    energy += result_map['SQ.e_leak']
    energy += result_map['LQ.wr_wire.e_w_eu2lsq_bb']
    energy += result_map['LQ.rd_wire.e_w_rr2lsq_bb']
    energy += result_map['LQ.rd_wire.e_w_cache2lsq_bb']
    energy += result_map['LQ.rd_wire.e_w_lsq2brob']
    energy += result_map['LQ.wr_wire.e_w_eu2lsq_o3']
    energy += result_map['LQ.rd_wire.e_w_rr2lsq_o3']
    energy += result_map['LQ.rd_wire.e_w_cache2lsq_o3']
    energy += result_map['LQ.rd_wire.e_w_lsq2rf']
    energy += result_map['LQ.e_cam']
    energy += result_map['LQ.e_ram']
    energy += result_map['LQ.e_leak']
    energy += result_map['dtlb.e_cam']
    energy += result_map['itlb.e_cam']
    energy += result_map['memory.e_ff']
    energy += result_map['memory.e_leak']
    energy += result_map['l1_d_0.e_ram']
    energy += result_map['l1_d_0.e_leak']

    # for now, combine L2 and L3 into memory
    energy += result_map['l2_0.e_ram']
    energy += result_map['l2_0.e_leak']
    energy += result_map['l3_0.e_ram']
    energy += result_map['l3_0.e_leak']

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['memory.epo'] = epo
    result_map['memory.power'] = energy
    result_map['memory'] = energy

def lsq_energy (result_map):
    energy = 0

    energy += result_map['stBuff.e_ram']
    energy += result_map['stBuff.e_leak']

    energy += result_map['SQ.e_cam']
    energy += result_map['SQ.e_ram']
    energy += result_map['SQ.e_leak']

    energy += result_map['LQ.e_cam']
    energy += result_map['LQ.e_ram']
    energy += result_map['LQ.e_leak']

    ins_cnt = result_map['memory.ins_cnt']

    energy /= float(ins_cnt)

    result_map['lsq.Energy'] = energy

def l2_l3_energy (result_map):
    energy = 0

    energy += result_map['l2_0.e_ram']
    energy += result_map['l2_0.e_leak']
    energy += result_map['l3_0.e_ram']
    energy += result_map['l3_0.e_leak']

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['l2-l3.epo'] = epo
    result_map['l2-l3.power'] = energy
    result_map['l2-l3'] = energy

def commit_energy (result_map):
    energy = 0

    energy += result_map['LQ.rd_wire.e_w_lsq2rob']
    energy += result_map['SQ.rd_wire.e_w_lsq2rob']
    energy += result_map['bbROB.wr_wire.e_w_eu2brob']
    energy += result_map['bbROB.wr_wire.e_w_rr2brob']
    energy += result_map['bbROB.rd_wire.e_wire']
    energy += result_map['bbROB.e_ram']
    energy += result_map['bbROB.e_leak']
    energy += result_map['iROB.rd_wire.e_w_eu2rob']
    energy += result_map['iROB.wr_wire.e_w_rr2rob']
    energy += result_map['iROB.e_ram']
    energy += result_map['iROB.e_leak']
    energy += result_map['commit.e_ff']
    energy += result_map['commit.e_leak']

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['commit.epo'] = epo
    result_map['commit.power'] = energy
    result_map['commit'] = energy

def rf_energy (result_map):
    energy = 0

    energy += result_map['rfManager.e_leak']
    energy += result_map['rfManager.e_ram']
    energy += result_map['rfManager.rf.e_leak']
    energy += result_map['rfManager.rf.e_ram']
    energy += result_map['grfManager.grf.e_leak']
    energy += result_map['grfManager.grf.e_ram']
    energy += result_map['lrfManager.e_leak']
    energy += result_map['lrfManager.e_ram']

    energy += result_map['registerFile.wr_wire.e_w_cache2rf']
    energy += result_map['registerFile.wr_wire.e_w_eu2simplerf']
    energy += result_map['registerFile.rd_wire.e_wire']
    energy += result_map['registerFile.wr_wire.e_wire']
    energy += result_map['registerRename.wr_wire.e_w_eu2rf']
    energy += result_map['GlobalRegisterRename.wr_wire.e_w_eu2grf']
    energy += result_map['LocalRegisterFile.wr_wire.e_w_eu2lrf']
    energy += result_map['LocalRegisterFile.rd_wire.e_wire']
    energy += result_map['LQ.rd_wire.e_w_lsq2lrf']
    energy += result_map['LQ.rd_wire.e_w_lsq2grf']

    cycle_time  = 0.5e-9 #second
    delay       = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']
    power       = energy / float(delay)
    epo         = energy / float(all_ops)

    result_map['rf.epo'] = epo
    result_map['rf.power'] = energy
    result_map['rf'] = energy

def wire_energy (result_map):
    energy = 0

    energy += result_map['2bc_gskew.wire.e_w_bp2cache_o3']
    energy += result_map['2bc_gskew.wire.e_w_bp2cache_ino']
    energy += result_map['2bc_gskew.wire.e_w_bp2cache_bb']
    energy += result_map['grfManager.rr.wire.e_w_cache2rr']
    energy += result_map['registerFile.rr.wire.e_w_cache2rr']
    energy += result_map['iWindow.wr_wire.e_w_cache2win']
    energy += result_map['iWindow.wr_wire.e_w_win2eu']
    energy += result_map['ResStn_0.wr_wire.e_w_rr2rs']
    energy += result_map['ResStn_0.rd_wire.e_w_rs2eu']
    energy += result_map['bbWindow.wr_wire.e_w_rr2iq']
    energy += result_map['bbWindow.rd_wire.e_w_iq2eu']
    energy += result_map['SQ.wr_wire.e_w_eu2lsq_bb']
    energy += result_map['SQ.rd_wire.e_w_rr2lsq_bb']
    energy += result_map['SQ.rd_wire.e_w_cache2lsq_bb']
    energy += result_map['SQ.rd_wire.e_w_lsq2brob']
    energy += result_map['SQ.wr_wire.e_w_eu2lsq_o3']
    energy += result_map['SQ.rd_wire.e_w_rr2lsq_o3']
    energy += result_map['SQ.rd_wire.e_w_cache2lsq_o3']
    energy += result_map['SQ.rd_wire.e_w_lsq2rob']
    energy += result_map['LQ.wr_wire.e_w_eu2lsq_bb']
    energy += result_map['LQ.rd_wire.e_w_rr2lsq_bb']
    energy += result_map['LQ.rd_wire.e_w_cache2lsq_bb']
    energy += result_map['LQ.rd_wire.e_w_lsq2brob']
    energy += result_map['LQ.wr_wire.e_w_eu2lsq_o3']
    energy += result_map['LQ.rd_wire.e_w_rr2lsq_o3']
    energy += result_map['LQ.rd_wire.e_w_cache2lsq_o3']
    energy += result_map['LQ.rd_wire.e_w_lsq2rf']
    energy += result_map['LQ.rd_wire.e_w_lsq2rob']
    energy += result_map['LQ.rd_wire.e_w_lsq2lrf']
    energy += result_map['LQ.rd_wire.e_w_lsq2grf']
    energy += result_map['bbROB.wr_wire.e_w_eu2brob']
    energy += result_map['bbROB.wr_wire.e_w_rr2brob']
    energy += result_map['iROB.rd_wire.e_w_eu2rob']
    energy += result_map['iROB.wr_wire.e_w_rr2rob']
    energy += result_map['registerRename.wr_wire.e_w_eu2rf']
    energy += result_map['GlobalRegisterRename.wr_wire.e_w_eu2grf']
    energy += result_map['LocalRegisterFile.wr_wire.e_w_eu2lrf']
    energy += result_map['registerFile.wr_wire.e_w_cache2rf']
    energy += result_map['registerFile.wr_wire.e_w_eu2simplerf']

    result_map['wire'] = energy

def stage_energy (result_map):
    energy = 0

    energy += result_map['fetch.e_leak']
    energy += result_map['branchPred.e_leak']
    energy += result_map['decode.e_leak']
    energy += result_map['schedule.e_leak']
    energy += result_map['execution.e_leak']
    energy += result_map['stBuff.e_ram']
    energy += result_map['stBuff.e_leak']
    energy += result_map['memory.e_leak']
    energy += result_map['commit.e_leak']
    energy += result_map['fetch.e_ff']
    energy += result_map['branchPred.e_ff']
    energy += result_map['decode.e_ff']
    energy += result_map['schedule.e_ff']
    energy += result_map['execution.e_ff']
    energy += result_map['stBuff.e_ram']
    energy += result_map['stBuff.e_leak']
    energy += result_map['memory.e_ff']
    energy += result_map['commit.e_ff']

    result_map['stage-reg'] = energy

# CORE ENERGY WITHOUT CACHE
def core_energy (result_map):
    energy = 0

    energy += result_map['TOTAL.Energy']
    energy -= result_map['l1_d_0.e_ram']
    energy -= result_map['l1_d_0.e_leak']
    energy -= result_map['l1_i_0.e_ram']
    energy -= result_map['l1_i_0.e_leak']
    energy -= result_map['l2_0.e_ram']
    energy -= result_map['l2_0.e_leak']
    energy -= result_map['l3_0.e_ram']
    energy -= result_map['l3_0.e_leak']

    result_map['core.Energy'] = energy

# POWER
def core_power (result_map):
    energy = 0
    cycle_time = 0.5e-9 #second
    delay = result_map['sysClock.clk_cycles'] * cycle_time
    all_ops     = result_map['commit.ins_cnt']

    energy += result_map['TOTAL.Energy']
    energy -= result_map['l1_d_0.e_ram']
    energy -= result_map['l1_d_0.e_leak']
    energy -= result_map['l1_i_0.e_ram']
    energy -= result_map['l1_i_0.e_leak']
    energy -= result_map['l2_0.e_ram']
    energy -= result_map['l2_0.e_leak']
    energy -= result_map['l3_0.e_ram']
    energy -= result_map['l3_0.e_leak']

    power = energy / delay
    result_map['core.Power'] = power
    result_map['core.epo'] = energy / float(all_ops)

def total_power (result_map):
    energy = 0
    cycle_time = 0.5e-9 #second
    delay = result_map['sysClock.clk_cycles'] * cycle_time

    energy += result_map['TOTAL.Energy']

    power = energy / float(delay)
    result_map['TOTAL.Power'] = power

    all_ops = result_map['commit.ins_cnt']
    epo = energy / float(all_ops)
    result_map['TOTAL.EPO'] = epo

def dynamic_power (result_map):
    energy = 0
    cycle_time = 0.5e-9 #second
    delay = result_map['sysClock.clk_cycles'] * cycle_time

    energy += result_map['DYNAMIC.Energy']

    power = energy / float(delay)
    result_map['DYNAMIC.Power'] = power

    all_ops = result_map['commit.ins_cnt']
    epo = energy / float(all_ops)
    result_map['DYNAMIC.epo'] = epo

def static_power (result_map):
    energy = 0
    cycle_time = 0.5e-9 #second
    delay = result_map['sysClock.clk_cycles'] * cycle_time

    energy += result_map['STATIC.Energy']

    power = energy / float(delay)
    result_map['STATIC.Power'] = power

    all_ops = result_map['commit.ins_cnt']
    epo = energy / float(all_ops)
    result_map['STATIC.epo'] = epo

# LRF / GRF
def lrf_energy (result_map):
    lrf_energy = 0

    lrf_energy += result_map['lrfManager.e_leak']
    lrf_energy += result_map['lrfManager.e_ram']

    all_ops = result_map['commit.ins_cnt']
    epo = lrf_energy / float(all_ops)

    result_map['lrf.Energy'] = lrf_energy
    result_map['lrf.epo'] = epo

def grf_energy (result_map):
    grf_energy = 0

    grf_energy += result_map['grfManager.grf.e_leak']
    grf_energy += result_map['grfManager.grf.e_ram']

    grf_energy += result_map['rfManager.rf.e_leak']
    grf_energy += result_map['rfManager.rf.e_ram']

    grf_energy += result_map['rfManager.e_leak']
    grf_energy += result_map['rfManager.e_ram']

    all_ops = result_map['commit.ins_cnt']
    epo = grf_energy / float(all_ops)

    result_map['grf.Energy'] = grf_energy
    result_map['grf.epo'] = epo

# ENERGY DELAY ANALYSIS
def ed (result_map):
    ed = 0.0
    cycle_time = 0.5e-9
    jouls = 1.0e-12
    energy = result_map['TOTAL.Energy'] * jouls
    delay = result_map['sysClock.clk_cycles'] * cycle_time
    ed = energy * delay

    result_map['ED'] = ed

def ed2 (result_map):
    ed2 = 0.0
    cycle_time = 0.5e-9 # second
    jouls = 1.0e-12 #pJ -> J
    energy = result_map['TOTAL.Energy'] * jouls
    delay = result_map['sysClock.clk_cycles'] * cycle_time
    ed2 = energy * delay * delay

    result_map['ED2'] = ed2

def bipsqw (result_map):
    bi = 0.0
    bips = 0.0
    bipsqw = 0.0
    billion = 1.0e9

    cycle_time = 0.5e-9 #second
    delay = result_map['sysClock.clk_cycles'] * cycle_time

    all_ops = result_map['commit.ins_cnt']
    bi = float(all_ops) / billion
    bips = float(bi) / delay
    bipsq = bips * bips * bips

    jouls = 1.0e-12 #pJ -> J
    energy = result_map['TOTAL.Energy'] * jouls
    power = energy / delay

    bipsqw = bipsq / power

    result_map['bipsqw'] = bipsqw

def joul_per_op (result_map):
    jpop = 0.0
    jouls = 1.0e-12 #pJ -> J
    energy = result_map['TOTAL.Energy'] * jouls
    all_ops = result_map['commit.ins_cnt']
    jpop = energy / float(all_ops)

    result_map['JPOp'] = jpop

# EU BUSY BREAKDOWN
def no_compute (result_map):
    result_map['no-compute'] = 1.0 - result_map['execution.ipc']
    
def mem_compute (result_map):
    mem_ops = result_map['commit.ins_type_cnt_2']
    all_ops = result_map['commit.ins_cnt']
    mem_rate = float(mem_ops) / all_ops
    result_map['mem-compute'] = result_map['execution.ipc'] * mem_rate
    
def alu_compute (result_map):
    alu_ops = result_map['commit.ins_type_cnt_1']
    br_ops = result_map['commit.ins_type_cnt_4']
    all_ops = result_map['commit.ins_cnt']
    alu_rate = float(alu_ops + br_ops) / all_ops
    result_map['alu-compute'] = result_map['execution.ipc'] * alu_rate

#SPECULATION ACCURACY
def br_accuracy (result_map):
    bp_mis_pred = result_map['pars.bp_misspred_cnt']
    btb_mis_pred = result_map['pars.btb_misspred_cnt']
#    bpu_lookup = result_map['pars.bpu_lookup_cnt']
    ins_cnt = result_map['commit.ins_cnt']

    accu = (bp_mis_pred + btb_mis_pred) / ins_cnt
    accu_per_1k = accu * 1000

    result_map['bpu-accuracy'] = accu_per_1k

#SPECULATION ACCURACY
def mem_accuracy (result_map):
    mem_mis_pred = result_map['execution.mem_mispred_cnt']
#    mem_lookup = result_map['memory.ins_cnt']
    ins_cnt = result_map['commit.ins_cnt']

    accu = mem_mis_pred / ins_cnt
    accu_per_1k = accu * 1000

    result_map['mem-accuracy'] = accu_per_1k

#MISC
def wasted_ins_rat (result_map):
    wasted_ins_cnt = result_map['commit.num_waste_ins']
    ins_cnt = result_map['commit.ins_cnt']
    rat = wasted_ins_cnt / ins_cnt
    rat_per_1k = rat * 1000

    result_map['wasted_ins_rat'] = rat_per_1k

# MEMORY
def cache_miss (result_map):
    cache_miss = result_map['lsqManager.cache_miss_cnt']
    cache_hit = result_map['lsqManager.cache_hit_cnt']
    if cache_miss == 0 or cache_hit == 0:
      cache_miss = result_map['memory.cache_miss_cnt']
      cache_hit = result_map['memory.cache_hit_cnt']
    cache_axes = cache_miss + cache_hit

    miss_rate = float(cache_miss) / cache_axes

    result_map['cache_miss_rat'] = miss_rate

def window_size (result_map):
    win_size = 0.0
    avg_bb_cnt = result_map['commit.bb_size_avg']
    avg_bb_size = result_map['schedule.bbWin_inflight_rat']
    res_stn_size = result_map['ResStn_0.size_rat']
    skipahead_size = 5

    win_size = avg_bb_size * skipahead_size + res_stn_size
    result_map['window_avg_aize'] = win_size
