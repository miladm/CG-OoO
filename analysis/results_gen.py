#!/usr/bin/env python
from __future__ import division
from collections import deque
from collections import defaultdict
from time import time
import os, glob, os.path
import sys
import re
import os
import results_verif

# INITIALIZE THE BENCHMARKS
def init_bench (results_table):
  b_names = ["400.perlbench", "401.bzip2", "403.gcc", "429.mcf", \
             "445.gobmk", "456.hmmer", "458.sjeng", "462.libquantum", \
             "464.h264ref", "471.omnetpp", "473.astar", "483.xalancbmk"]
  for b_name in b_names:
    results_table['bench_names'].append (b_name)
  return b_names

# INITIALIZE FILE NAMES AND OTHER SETUP ATTRIBUTES
def init_files (b_names, mode, result_in_dir):
  f_names = []
  in_paths = []
  for b_name in b_names:
    f_name = b_name + '_' + mode
    f_names.append (f_name)
  for f_name in f_names:
    in_path = result_in_dir + '/' + f_name
    in_paths.append (in_path)
  return in_paths

# PRINT THE OUTPUT HEADER
def print_headers (result_out_dir, results_table):
  for result_param in results_table:
    if result_param == 'bench_names': continue
    bench_names = results_table['bench_names']
    out_path = result_out_dir + '/' + result_param + '.csv'
    if os.path.isfile (out_path): break
    pF_out = open (out_path, 'a')
    pF_out.write (' ' + ', ')
    for value in bench_names:
      pF_out.write (str (value) + ', ')
    pF_out.write ('\n')
    pF_out.close ()
    print out_path

# PRINT THE RESULTS 
def print_results_table (result_out_dir, results_table, mode):
  for result_param in results_table:
    if result_param == 'bench_names': continue
    result_values = results_table[result_param]
    out_path = result_out_dir + '/' + result_param + '.csv'
    pF_out = open (out_path, 'a')
    pF_out.write (mode + ', ')
    for value in result_values:
      pF_out.write (str (value) + ', ')
    pF_out.write ('\n')
    pF_out.close ()
    print out_path

# INITIALIZE THE MAP BETWEEN ENERGY AND PERFORMANCE PARAMTERS
def init_params (result_map):
  # FOR ENERGY MEASUREMENT
  result_map['STATIC.Energy'] = 0.0
  result_map['DYNAMIC.Energy'] = 0.0
  result_map['TOTAL.Energy'] = 0.0
  result_map['EU.e_eu'] = 0.0
  result_map['stBuff.e_ram'] = 0.0
  result_map['stBuff.e_leak'] = 0.0
  result_map['iWindow.e_leak'] = 0.0
  result_map['iWindow.e_ram'] = 0.0
  result_map['iWindow.rd_wire.e_wire'] = 0.0
  result_map['iWindow.wr_wire.e_w_cache2win'] = 0.0
  result_map['iWindow.wr_wire.e_w_win2eu'] = 0.0
  result_map['iROB.rd_wire.e_w_eu2rob'] = 0.0
  result_map['iROB.wr_wire.e_w_rr2rob'] = 0.0
  result_map['iROB.e_ram'] = 0.0
  result_map['iROB.e_leak'] = 0.0
  result_map['rfManager.e_ram'] = 0.0
  result_map['registerFile.rd_wire.e_wire'] = 0.0
  result_map['registerFile.wr_wire.e_wire'] = 0.0
  result_map['registerFile.rr.wire.e_w_cache2rr'] = 0.0
  result_map['registerFile.wr_wire.e_w_cache2rf'] = 0.0
  result_map['registerFile.wr_wire.e_w_eu2simplerf'] = 0.0
  result_map['BTB.e_cam'] = 0.0
  result_map['2bc_gskew.e_ram'] = 0.0
  result_map['2bc_gskew.e_leak'] = 0.0
  result_map['2bc_gskew.wire.e_w_bp2cache_o3'] = 0.0
  result_map['2bc_gskew.wire.e_w_bp2cache_ino'] = 0.0
  result_map['2bc_gskew.wire.e_w_bp2cache_bb'] = 0.0
  result_map['registerRename.wr_wire.e_w_eu2rf'] = 0.0
  result_map['registerRename.rd_wire.e_wire'] = 0.0
  result_map['rfManager.rf.e_leak'] = 0.0
  result_map['rfManager.rf.e_ram'] = 0.0
  result_map['rfManager.rat.e_ram'] = 0.0
  result_map['rfManager.apr.e_ram'] = 0.0
  result_map['rfManager.arst.e_ram'] = 0.0
  result_map['SQ.wr_wire.e_w_eu2lsq_bb'] = 0.0
  result_map['SQ.rd_wire.e_w_rr2lsq_bb'] = 0.0
  result_map['SQ.rd_wire.e_w_cache2lsq_bb'] = 0.0
  result_map['SQ.rd_wire.e_w_lsq2brob'] = 0.0
  result_map['SQ.wr_wire.e_w_eu2lsq_o3'] = 0.0
  result_map['SQ.rd_wire.e_w_rr2lsq_o3'] = 0.0
  result_map['SQ.rd_wire.e_w_cache2lsq_o3'] = 0.0
  result_map['SQ.rd_wire.e_w_lsq2rob'] = 0.0
  result_map['SQ.e_cam'] = 0.0
  result_map['SQ.e_ram'] = 0.0
  result_map['SQ.e_leak'] = 0.0
  result_map['LQ.wr_wire.e_w_eu2lsq_bb'] = 0.0
  result_map['LQ.rd_wire.e_w_rr2lsq_bb'] = 0.0
  result_map['LQ.rd_wire.e_w_cache2lsq_bb'] = 0.0
  result_map['LQ.rd_wire.e_w_lsq2brob'] = 0.0
  result_map['LQ.wr_wire.e_w_eu2lsq_o3'] = 0.0
  result_map['LQ.rd_wire.e_w_rr2lsq_o3'] = 0.0
  result_map['LQ.rd_wire.e_w_cache2lsq_o3'] = 0.0
  result_map['LQ.rd_wire.e_w_lsq2rf'] = 0.0
  result_map['LQ.rd_wire.e_w_lsq2rob'] = 0.0
  result_map['LQ.e_cam'] = 0.0
  result_map['LQ.e_ram'] = 0.0
  result_map['LQ.e_leak'] = 0.0
  result_map['dtlb.e_cam'] = 0.0
  result_map['ResStn_0.wr_wire.e_w_rr2rs'] = 0.0
  result_map['ResStn_0.rd_wire.e_w_rs2eu'] = 0.0
  result_map['ResStn_0.e_leak'] = 0.0
  result_map['ResStn_0.e_cam'] = 0.0
  result_map['ResStn_0.e_cam2'] = 0.0
  result_map['ResStn_0.e_ram'] = 0.0
  result_map['GlobalRegisterRename.wr_wire.e_w_eu2grf'] = 0.0
  result_map['GlobalRegisterRename.rd_wire.e_wire'] = 0.0
  result_map['grfManager.rr.wire.e_w_cache2rr'] = 0.0
  result_map['grfManager.grf.e_leak'] = 0.0
  result_map['grfManager.grf.e_ram'] = 0.0
  result_map['grfManager.grat.e_ram'] = 0.0
  result_map['grfManager.gapr.e_ram'] = 0.0
  result_map['grfManager.garst.e_ram'] = 0.0
  result_map['bbROB.wr_wire.e_w_eu2brob'] = 0.0
  result_map['bbROB.wr_wire.e_w_rr2brob'] = 0.0
  result_map['bbROB.rd_wire.e_wire'] = 0.0
  result_map['bbROB.e_ram'] = 0.0
  result_map['bbROB.e_leak'] = 0.0
  result_map['bbWindow.wr_wire.e_w_rr2iq'] = 0.0
  result_map['bbWindow.rd_wire.e_w_iq2eu'] = 0.0
  result_map['bbWindow.e_cam'] = 0.0
  result_map['bbWindow.e_ram'] = 0.0
  result_map['bbWindow.e_leak'] = 0.0
  result_map['lrfManager.e_leak'] = 0.0
  result_map['lrfManager.e_ram'] = 0.0
  result_map['LocalRegisterFile.wr_wire.e_w_eu2lrf'] = 0.0
  result_map['LocalRegisterFile.rd_wire.e_wire'] = 0.0
  # CACHE ENERGY
  result_map['l1_d_0.e_ram'] = 0.0
  result_map['l1_d_0.e_leak'] = 0.0
  result_map['l1_i_0.e_ram'] = 0.0
  result_map['l1_i_0.e_leak'] = 0.0
  result_map['l2_0.e_ram'] = 0.0
  result_map['l2_0.e_leak'] = 0.0
  result_map['l3_0.e_ram'] = 0.0
  result_map['l3_0.e_leak'] = 0.0
  # STAGE ENERGY COMPONENTS
  result_map['branchPred.e_ff'] = 0.0
  result_map['branchPred.e_leak'] = 0.0
  result_map['fetch.e_ff'] = 0.0
  result_map['fetch.e_leak'] = 0.0
  result_map['decode.e_ff'] = 0.0
  result_map['decode.e_leak'] = 0.0
  result_map['schedule.e_ff'] = 0.0
  result_map['schedule.e_leak'] = 0.0
  result_map['execution.e_ff'] = 0.0
  result_map['execution.e_leak'] = 0.0
  result_map['memory.e_ff'] = 0.0
  result_map['memory.e_leak'] = 0.0
  result_map['commit.e_ff'] = 0.0
  result_map['commit.e_leak'] = 0.0
  result_map['schedule.ins_cluster_hist_0'] = 0.0
  result_map['schedule.ins_cluster_hist_1'] = 0.0
  result_map['schedule.ins_cluster_hist_2'] = 0.0
  result_map['schedule.ins_cluster_hist_3'] = 0.0
  result_map['schedule.ins_cluster_hist_4'] = 0.0
  result_map['schedule.ins_cluster_hist_5'] = 0.0
  result_map['schedule.ins_cluster_hist_6'] = 0.0
  result_map['schedule.ins_cluster_hist_7'] = 0.0
  result_map['schedule.ins_cluster_hist_8'] = 0.0
  result_map['schedule.ins_cluster_hist_9'] = 0.0
  result_map['schedule.ins_cluster_hist_10'] = 0.0
  result_map['schedule.ins_cluster_hist_11'] = 0.0
  result_map['schedule.ins_cluster_hist_12'] = 0.0
  result_map['schedule.ins_cluster_hist_13'] = 0.0
  result_map['schedule.ins_cluster_hist_14'] = 0.0
  result_map['schedule.ins_cluster_hist_15'] = 0.0
  result_map['schedule.ins_cluster_hist_16'] = 0.0
  result_map['schedule.ins_cluster_hist_17'] = 0.0
  result_map['schedule.ins_cluster_hist_18'] = 0.0
  result_map['schedule.ins_cluster_hist_19'] = 0.0
  result_map['schedule.ins_cluster_hist_20'] = 0.0
  result_map['schedule.ins_cluster_hist_21'] = 0.0
  result_map['schedule.ins_cluster_hist_22'] = 0.0
  result_map['schedule.ins_cluster_hist_23'] = 0.0
  result_map['schedule.ins_cluster_hist_24'] = 0.0
  result_map['schedule.ins_cluster_hist_25'] = 0.0
  result_map['schedule.ins_cluster_hist_26'] = 0.0
  result_map['schedule.ins_cluster_hist_27'] = 0.0
  result_map['schedule.ins_cluster_hist_28'] = 0.0
  result_map['schedule.ins_cluster_hist_29'] = 0.0
  result_map['schedule.ins_cluster_hist_30'] = 0.0
  result_map['schedule.ins_cluster_hist_31'] = 0.0
  result_map['schedule.ins_cluster_hist_32'] = 0.0
  result_map['schedule.ins_cluster_hist_33'] = 0.0
  result_map['schedule.ins_cluster_hist_34'] = 0.0
  result_map['schedule.ins_cluster_hist_35'] = 0.0
  result_map['schedule.ins_cluster_hist_36'] = 0.0
  result_map['schedule.ins_cluster_hist_37'] = 0.0
  result_map['schedule.ins_cluster_hist_38'] = 0.0
  result_map['schedule.ins_cluster_hist_39'] = 0.0
  result_map['schedule.ins_cluster_hist_40'] = 0.0
  result_map['schedule.ins_cluster_hist_41'] = 0.0
  result_map['schedule.ins_cluster_hist_42'] = 0.0
  result_map['schedule.ins_cluster_hist_43'] = 0.0
  result_map['schedule.ins_cluster_hist_44'] = 0.0
  result_map['schedule.ins_cluster_hist_45'] = 0.0
  result_map['schedule.ins_cluster_hist_46'] = 0.0
  result_map['schedule.ins_cluster_hist_47'] = 0.0
  result_map['schedule.ins_cluster_hist_48'] = 0.0
  result_map['schedule.ins_cluster_hist_49'] = 0.0
  result_map['schedule.ins_cluster_hist_50'] = 0.0
  result_map['schedule.ins_cluster_hist_51'] = 0.0
  result_map['schedule.ins_cluster_hist_52'] = 0.0
  result_map['schedule.ins_cluster_hist_53'] = 0.0
  result_map['schedule.ins_cluster_hist_54'] = 0.0
  result_map['schedule.ins_cluster_hist_55'] = 0.0
  result_map['schedule.ins_cluster_hist_56'] = 0.0
  result_map['schedule.ins_cluster_hist_57'] = 0.0
  result_map['schedule.ins_cluster_hist_58'] = 0.0
  result_map['schedule.ins_cluster_hist_59'] = 0.0
  result_map['schedule.ins_cluster_hist_60'] = 0.0
  result_map['schedule.ins_cluster_hist_61'] = 0.0
  result_map['schedule.ins_cluster_hist_62'] = 0.0
  result_map['schedule.ins_cluster_hist_63'] = 0.0
  result_map['schedule.ins_cluster_hist_64'] = 0.0
  result_map['schedule.ins_cluster_hist_65'] = 0.0
  result_map['schedule.ins_cluster_hist_66'] = 0.0
  result_map['schedule.ins_cluster_hist_67'] = 0.0
  result_map['schedule.ins_cluster_hist_68'] = 0.0
  result_map['schedule.ins_cluster_hist_69'] = 0.0
  result_map['schedule.ins_cluster_hist_70'] = 0.0
  result_map['schedule.ins_cluster_hist_71'] = 0.0
  result_map['schedule.ins_cluster_hist_72'] = 0.0
  result_map['schedule.ins_cluster_hist_73'] = 0.0
  result_map['schedule.ins_cluster_hist_74'] = 0.0
  result_map['schedule.ins_cluster_hist_75'] = 0.0
  result_map['schedule.ins_cluster_hist_76'] = 0.0
  result_map['schedule.ins_cluster_hist_77'] = 0.0
  result_map['schedule.ins_cluster_hist_78'] = 0.0
  result_map['schedule.ins_cluster_hist_79'] = 0.0
  result_map['schedule.ins_cluster_hist_80'] = 0.0
  result_map['schedule.ins_cluster_hist_81'] = 0.0
  result_map['schedule.ins_cluster_hist_82'] = 0.0
  result_map['schedule.ins_cluster_hist_83'] = 0.0
  result_map['schedule.ins_cluster_hist_84'] = 0.0
  result_map['schedule.ins_cluster_hist_85'] = 0.0
  result_map['schedule.ins_cluster_hist_86'] = 0.0
  result_map['schedule.ins_cluster_hist_87'] = 0.0
  result_map['schedule.ins_cluster_hist_88'] = 0.0
  result_map['schedule.ins_cluster_hist_89'] = 0.0
  result_map['schedule.ins_cluster_hist_90'] = 0.0
  result_map['schedule.ins_cluster_hist_91'] = 0.0
  result_map['schedule.ins_cluster_hist_92'] = 0.0
  result_map['schedule.ins_cluster_hist_93'] = 0.0
  result_map['schedule.ins_cluster_hist_94'] = 0.0
  result_map['schedule.ins_cluster_hist_95'] = 0.0
  result_map['schedule.ins_cluster_hist_96'] = 0.0
  result_map['schedule.ins_cluster_hist_97'] = 0.0
  result_map['schedule.ins_cluster_hist_98'] = 0.0
  result_map['schedule.ins_cluster_hist_99'] = 0.0
  result_map['schedule.ins_cluster_hist_100'] = 0.0
  result_map['schedule.ins_cluster_hist_101'] = 0.0
  result_map['schedule.ins_cluster_hist_102'] = 0.0
  result_map['schedule.ins_cluster_hist_103'] = 0.0
  result_map['schedule.ins_cluster_hist_104'] = 0.0
  result_map['schedule.ins_cluster_hist_105'] = 0.0
  result_map['schedule.ins_cluster_hist_106'] = 0.0
  result_map['schedule.ins_cluster_hist_107'] = 0.0
  result_map['schedule.ins_cluster_hist_108'] = 0.0
  result_map['schedule.ins_cluster_hist_109'] = 0.0
  result_map['schedule.ins_cluster_hist_110'] = 0.0
  result_map['schedule.ins_cluster_hist_111'] = 0.0
  result_map['schedule.ins_cluster_hist_112'] = 0.0
  result_map['schedule.ins_cluster_hist_113'] = 0.0
  result_map['schedule.ins_cluster_hist_114'] = 0.0
  result_map['schedule.ins_cluster_hist_115'] = 0.0
  result_map['schedule.ins_cluster_hist_116'] = 0.0
  result_map['schedule.ins_cluster_hist_117'] = 0.0
  result_map['schedule.ins_cluster_hist_118'] = 0.0
  result_map['schedule.ins_cluster_hist_119'] = 0.0
  result_map['schedule.ins_cluster_hist_120'] = 0.0
  result_map['schedule.ins_cluster_hist_121'] = 0.0
  result_map['schedule.ins_cluster_hist_122'] = 0.0
  result_map['schedule.ins_cluster_hist_123'] = 0.0
  result_map['schedule.ins_cluster_hist_124'] = 0.0
  result_map['schedule.ins_cluster_hist_125'] = 0.0
  result_map['schedule.ins_cluster_hist_126'] = 0.0
  result_map['schedule.ins_cluster_hist_127'] = 0.0
  result_map['schedule.ins_cluster_hist_128'] = 0.0
  result_map['schedule.ins_cluster_hist_129'] = 0.0
  result_map['schedule.ins_cluster_hist_130'] = 0.0
  result_map['schedule.ins_cluster_hist_131'] = 0.0
  result_map['schedule.ins_cluster_hist_132'] = 0.0
  result_map['schedule.ins_cluster_hist_133'] = 0.0
  result_map['schedule.ins_cluster_hist_134'] = 0.0
  result_map['schedule.ins_cluster_hist_135'] = 0.0
  result_map['schedule.ins_cluster_hist_136'] = 0.0
  result_map['schedule.ins_cluster_hist_137'] = 0.0
  result_map['schedule.ins_cluster_hist_138'] = 0.0
  result_map['schedule.ins_cluster_hist_139'] = 0.0
  result_map['schedule.ins_cluster_hist_140'] = 0.0
  result_map['schedule.ins_cluster_hist_141'] = 0.0
  result_map['schedule.ins_cluster_hist_142'] = 0.0
  result_map['schedule.ins_cluster_hist_143'] = 0.0
  result_map['schedule.ins_cluster_hist_144'] = 0.0
  result_map['schedule.ins_cluster_hist_145'] = 0.0
  result_map['schedule.ins_cluster_hist_146'] = 0.0
  result_map['schedule.ins_cluster_hist_147'] = 0.0
  result_map['schedule.ins_cluster_hist_148'] = 0.0
  result_map['schedule.ins_cluster_hist_149'] = 0.0
  result_map['schedule.ins_cluster_hist_150'] = 0.0
  result_map['schedule.ins_cluster_hist_151'] = 0.0
  result_map['schedule.ins_cluster_hist_152'] = 0.0
  result_map['schedule.ins_cluster_hist_153'] = 0.0
  result_map['schedule.ins_cluster_hist_154'] = 0.0
  result_map['schedule.ins_cluster_hist_155'] = 0.0
  result_map['schedule.ins_cluster_hist_156'] = 0.0
  result_map['schedule.ins_cluster_hist_157'] = 0.0
  result_map['schedule.ins_cluster_hist_158'] = 0.0
  result_map['schedule.ins_cluster_hist_159'] = 0.0
  result_map['schedule.ins_cluster_hist_160'] = 0.0
  result_map['schedule.ins_cluster_hist_161'] = 0.0
  result_map['schedule.ins_cluster_hist_162'] = 0.0
  result_map['schedule.ins_cluster_hist_163'] = 0.0
  result_map['schedule.ins_cluster_hist_164'] = 0.0
  result_map['schedule.ins_cluster_hist_165'] = 0.0
  result_map['schedule.ins_cluster_hist_166'] = 0.0
  result_map['schedule.ins_cluster_hist_167'] = 0.0
  result_map['schedule.ins_cluster_hist_168'] = 0.0
  result_map['schedule.ins_cluster_hist_169'] = 0.0
  result_map['schedule.ins_cluster_hist_170'] = 0.0
  result_map['schedule.ins_cluster_hist_171'] = 0.0
  result_map['schedule.ins_cluster_hist_172'] = 0.0
  result_map['schedule.ins_cluster_hist_173'] = 0.0
  result_map['schedule.ins_cluster_hist_174'] = 0.0
  result_map['schedule.ins_cluster_hist_175'] = 0.0
  result_map['schedule.ins_cluster_hist_176'] = 0.0
  result_map['schedule.ins_cluster_hist_177'] = 0.0
  result_map['schedule.ins_cluster_hist_178'] = 0.0
  result_map['schedule.ins_cluster_hist_179'] = 0.0
  result_map['schedule.ins_cluster_hist_180'] = 0.0
  result_map['schedule.ins_cluster_hist_181'] = 0.0
  result_map['schedule.ins_cluster_hist_182'] = 0.0
  result_map['schedule.ins_cluster_hist_183'] = 0.0
  result_map['schedule.ins_cluster_hist_184'] = 0.0
  result_map['schedule.ins_cluster_hist_185'] = 0.0
  result_map['schedule.ins_cluster_hist_186'] = 0.0
  result_map['schedule.ins_cluster_hist_187'] = 0.0
  result_map['schedule.ins_cluster_hist_188'] = 0.0
  result_map['schedule.ins_cluster_hist_189'] = 0.0
  result_map['schedule.ins_cluster_hist_190'] = 0.0
  result_map['schedule.ins_cluster_hist_191'] = 0.0
  result_map['schedule.ins_cluster_hist_192'] = 0.0
  result_map['schedule.ins_cluster_hist_193'] = 0.0
  result_map['schedule.ins_cluster_hist_194'] = 0.0
  result_map['schedule.ins_cluster_hist_195'] = 0.0
  result_map['schedule.ins_cluster_hist_196'] = 0.0
  result_map['schedule.ins_cluster_hist_197'] = 0.0
  result_map['schedule.ins_cluster_hist_198'] = 0.0
  result_map['schedule.ins_cluster_hist_199'] = 0.0
  result_map['schedule.ins_cluster_hist_200'] = 0.0
  result_map['schedule.ins_cluster_hist_201'] = 0.0
  result_map['schedule.ins_cluster_hist_202'] = 0.0
  result_map['schedule.ins_cluster_hist_203'] = 0.0
  result_map['schedule.ins_cluster_hist_204'] = 0.0
  result_map['schedule.ins_cluster_hist_205'] = 0.0
  result_map['schedule.ins_cluster_hist_206'] = 0.0
  result_map['schedule.ins_cluster_hist_207'] = 0.0
  result_map['schedule.ins_cluster_hist_208'] = 0.0
  result_map['schedule.ins_cluster_hist_209'] = 0.0
  result_map['schedule.ins_cluster_hist_210'] = 0.0
  result_map['schedule.ins_cluster_hist_211'] = 0.0
  result_map['schedule.ins_cluster_hist_212'] = 0.0
  result_map['schedule.ins_cluster_hist_213'] = 0.0
  result_map['schedule.ins_cluster_hist_214'] = 0.0
  result_map['schedule.ins_cluster_hist_215'] = 0.0
  result_map['schedule.ins_cluster_hist_216'] = 0.0
  result_map['schedule.ins_cluster_hist_217'] = 0.0
  result_map['schedule.ins_cluster_hist_218'] = 0.0
  result_map['schedule.ins_cluster_hist_219'] = 0.0
  result_map['schedule.ins_cluster_hist_220'] = 0.0
  result_map['schedule.ins_cluster_hist_221'] = 0.0
  result_map['schedule.ins_cluster_hist_222'] = 0.0
  result_map['schedule.ins_cluster_hist_223'] = 0.0
  result_map['schedule.ins_cluster_hist_224'] = 0.0
  result_map['schedule.ins_cluster_hist_225'] = 0.0
  result_map['schedule.ins_cluster_hist_226'] = 0.0
  result_map['schedule.ins_cluster_hist_227'] = 0.0
  result_map['schedule.ins_cluster_hist_228'] = 0.0
  result_map['schedule.ins_cluster_hist_229'] = 0.0
  result_map['schedule.ins_cluster_hist_230'] = 0.0
  result_map['schedule.ins_cluster_hist_231'] = 0.0
  result_map['schedule.ins_cluster_hist_232'] = 0.0
  result_map['schedule.ins_cluster_hist_233'] = 0.0
  result_map['schedule.ins_cluster_hist_234'] = 0.0
  result_map['schedule.ins_cluster_hist_235'] = 0.0
  result_map['schedule.ins_cluster_hist_236'] = 0.0
  result_map['schedule.ins_cluster_hist_237'] = 0.0
  result_map['schedule.ins_cluster_hist_238'] = 0.0
  result_map['schedule.ins_cluster_hist_239'] = 0.0
  result_map['schedule.ins_cluster_hist_240'] = 0.0
  result_map['schedule.ins_cluster_hist_241'] = 0.0
  result_map['schedule.ins_cluster_hist_242'] = 0.0
  result_map['schedule.ins_cluster_hist_243'] = 0.0
  result_map['schedule.ins_cluster_hist_244'] = 0.0
  result_map['schedule.ins_cluster_hist_245'] = 0.0
  result_map['schedule.ins_cluster_hist_246'] = 0.0
  result_map['schedule.ins_cluster_hist_247'] = 0.0
  result_map['schedule.ins_cluster_hist_248'] = 0.0
  result_map['schedule.ins_cluster_hist_249'] = 0.0
  result_map['schedule.ins_cluster_hist_250'] = 0.0
  result_map['schedule.ins_cluster_hist_251'] = 0.0
  result_map['schedule.ins_cluster_hist_252'] = 0.0
  result_map['schedule.ins_cluster_hist_253'] = 0.0
  result_map['schedule.ins_cluster_hist_254'] = 0.0
  result_map['schedule.ins_cluster_hist_255'] = 0.0
  result_map['schedule.ins_cluster_hist_256'] = 0.0
  result_map['schedule.ins_cluster_hist_257'] = 0.0
  result_map['schedule.ins_cluster_hist_258'] = 0.0
  result_map['schedule.ins_cluster_hist_259'] = 0.0
  result_map['schedule.ins_cluster_hist_260'] = 0.0
  result_map['schedule.ins_cluster_hist_261'] = 0.0
  result_map['schedule.ins_cluster_hist_262'] = 0.0
  result_map['schedule.ins_cluster_hist_263'] = 0.0
  result_map['schedule.ins_cluster_hist_264'] = 0.0
  result_map['schedule.ins_cluster_hist_265'] = 0.0
  result_map['schedule.ins_cluster_hist_266'] = 0.0
  result_map['schedule.ins_cluster_hist_267'] = 0.0
  result_map['schedule.ins_cluster_hist_268'] = 0.0
  result_map['schedule.ins_cluster_hist_269'] = 0.0
  result_map['schedule.ins_cluster_hist_270'] = 0.0
  result_map['schedule.ins_cluster_hist_271'] = 0.0
  result_map['schedule.ins_cluster_hist_272'] = 0.0
  result_map['schedule.ins_cluster_hist_273'] = 0.0
  result_map['schedule.ins_cluster_hist_274'] = 0.0
  result_map['schedule.ins_cluster_hist_275'] = 0.0
  result_map['schedule.ins_cluster_hist_276'] = 0.0
  result_map['schedule.ins_cluster_hist_277'] = 0.0
  result_map['schedule.ins_cluster_hist_278'] = 0.0
  result_map['schedule.ins_cluster_hist_279'] = 0.0
  result_map['schedule.ins_cluster_hist_280'] = 0.0
  result_map['schedule.ins_cluster_hist_281'] = 0.0
  result_map['schedule.ins_cluster_hist_282'] = 0.0
  result_map['schedule.ins_cluster_hist_283'] = 0.0
  result_map['schedule.ins_cluster_hist_284'] = 0.0
  result_map['schedule.ins_cluster_hist_285'] = 0.0
  result_map['schedule.ins_cluster_hist_286'] = 0.0
  result_map['schedule.ins_cluster_hist_287'] = 0.0
  result_map['schedule.ins_cluster_hist_288'] = 0.0
  result_map['schedule.ins_cluster_hist_289'] = 0.0
  result_map['schedule.ins_cluster_hist_290'] = 0.0
  result_map['schedule.ins_cluster_hist_291'] = 0.0
  result_map['schedule.ins_cluster_hist_292'] = 0.0
  result_map['schedule.ins_cluster_hist_293'] = 0.0
  result_map['schedule.ins_cluster_hist_294'] = 0.0
  result_map['schedule.ins_cluster_hist_295'] = 0.0
  result_map['schedule.ins_cluster_hist_296'] = 0.0
  result_map['schedule.ins_cluster_hist_297'] = 0.0
  result_map['schedule.ins_cluster_hist_298'] = 0.0
  result_map['schedule.ins_cluster_hist_299'] = 0.0
  result_map['schedule.ins_cluster_hist_300'] = 0.0
  result_map['schedule.ins_cluster_hist_301'] = 0.0
  result_map['schedule.ins_cluster_hist_302'] = 0.0
  result_map['schedule.ins_cluster_hist_303'] = 0.0
  result_map['schedule.ins_cluster_hist_304'] = 0.0
  result_map['schedule.ins_cluster_hist_305'] = 0.0
  result_map['schedule.ins_cluster_hist_306'] = 0.0
  result_map['schedule.ins_cluster_hist_307'] = 0.0
  result_map['schedule.ins_cluster_hist_308'] = 0.0
  result_map['schedule.ins_cluster_hist_309'] = 0.0
  result_map['schedule.ins_cluster_hist_310'] = 0.0
  result_map['schedule.ins_cluster_hist_311'] = 0.0
  result_map['schedule.ins_cluster_hist_312'] = 0.0
  result_map['schedule.ins_cluster_hist_313'] = 0.0
  result_map['schedule.ins_cluster_hist_314'] = 0.0
  result_map['schedule.ins_cluster_hist_315'] = 0.0
  result_map['schedule.ins_cluster_hist_316'] = 0.0
  result_map['schedule.ins_cluster_hist_317'] = 0.0
  result_map['schedule.ins_cluster_hist_318'] = 0.0
  result_map['schedule.ins_cluster_hist_319'] = 0.0
  result_map['schedule.ins_cluster_hist_320'] = 0.0
  result_map['schedule.ins_cluster_hist_321'] = 0.0
  result_map['schedule.ins_cluster_hist_322'] = 0.0
  result_map['schedule.ins_cluster_hist_323'] = 0.0
  result_map['schedule.ins_cluster_hist_324'] = 0.0
  result_map['schedule.ins_cluster_hist_325'] = 0.0
  result_map['schedule.ins_cluster_hist_326'] = 0.0
  result_map['schedule.ins_cluster_hist_327'] = 0.0
  result_map['schedule.ins_cluster_hist_328'] = 0.0
  result_map['schedule.ins_cluster_hist_329'] = 0.0
  result_map['schedule.ins_cluster_hist_330'] = 0.0
  result_map['schedule.ins_cluster_hist_331'] = 0.0
  result_map['schedule.ins_cluster_hist_332'] = 0.0
  result_map['schedule.ins_cluster_hist_333'] = 0.0
  result_map['schedule.ins_cluster_hist_334'] = 0.0
  result_map['schedule.ins_cluster_hist_335'] = 0.0
  result_map['schedule.ins_cluster_hist_336'] = 0.0
  result_map['schedule.ins_cluster_hist_337'] = 0.0
  result_map['schedule.ins_cluster_hist_338'] = 0.0
  result_map['schedule.ins_cluster_hist_339'] = 0.0
  result_map['schedule.ins_cluster_hist_340'] = 0.0
  result_map['schedule.ins_cluster_hist_341'] = 0.0
  result_map['schedule.ins_cluster_hist_342'] = 0.0
  result_map['schedule.ins_cluster_hist_343'] = 0.0
  result_map['schedule.ins_cluster_hist_344'] = 0.0
  result_map['schedule.ins_cluster_hist_345'] = 0.0
  result_map['schedule.ins_cluster_hist_346'] = 0.0
  result_map['schedule.ins_cluster_hist_347'] = 0.0
  result_map['schedule.ins_cluster_hist_348'] = 0.0
  result_map['schedule.ins_cluster_hist_349'] = 0.0
  result_map['schedule.ins_cluster_hist_350'] = 0.0
  result_map['schedule.ins_cluster_hist_351'] = 0.0
  result_map['schedule.ins_cluster_hist_352'] = 0.0
  result_map['schedule.ins_cluster_hist_353'] = 0.0
  result_map['schedule.ins_cluster_hist_354'] = 0.0
  result_map['schedule.ins_cluster_hist_355'] = 0.0
  result_map['schedule.ins_cluster_hist_356'] = 0.0
  result_map['schedule.ins_cluster_hist_357'] = 0.0
  result_map['schedule.ins_cluster_hist_358'] = 0.0
  result_map['schedule.ins_cluster_hist_359'] = 0.0
  result_map['schedule.ins_cluster_hist_360'] = 0.0
  result_map['schedule.ins_cluster_hist_361'] = 0.0
  result_map['schedule.ins_cluster_hist_362'] = 0.0
  result_map['schedule.ins_cluster_hist_363'] = 0.0
  result_map['schedule.ins_cluster_hist_364'] = 0.0
  result_map['schedule.ins_cluster_hist_365'] = 0.0
  result_map['schedule.ins_cluster_hist_366'] = 0.0
  result_map['schedule.ins_cluster_hist_367'] = 0.0
  result_map['schedule.ins_cluster_hist_368'] = 0.0
  result_map['schedule.ins_cluster_hist_369'] = 0.0
  result_map['schedule.ins_cluster_hist_370'] = 0.0
  result_map['schedule.ins_cluster_hist_371'] = 0.0
  result_map['schedule.ins_cluster_hist_372'] = 0.0
  result_map['schedule.ins_cluster_hist_373'] = 0.0
  result_map['schedule.ins_cluster_hist_374'] = 0.0
  result_map['schedule.ins_cluster_hist_375'] = 0.0
  result_map['schedule.ins_cluster_hist_376'] = 0.0
  result_map['schedule.ins_cluster_hist_377'] = 0.0
  result_map['schedule.ins_cluster_hist_378'] = 0.0
  result_map['schedule.ins_cluster_hist_379'] = 0.0
  result_map['schedule.ins_cluster_hist_380'] = 0.0
  result_map['schedule.ins_cluster_hist_381'] = 0.0
  result_map['schedule.ins_cluster_hist_382'] = 0.0
  result_map['schedule.ins_cluster_hist_383'] = 0.0
  result_map['schedule.ins_cluster_hist_384'] = 0.0
  result_map['schedule.ins_cluster_hist_385'] = 0.0
  result_map['schedule.ins_cluster_hist_386'] = 0.0
  result_map['schedule.ins_cluster_hist_387'] = 0.0
  result_map['schedule.ins_cluster_hist_388'] = 0.0
  result_map['schedule.ins_cluster_hist_389'] = 0.0
  result_map['schedule.ins_cluster_hist_390'] = 0.0
  result_map['schedule.ins_cluster_hist_391'] = 0.0
  result_map['schedule.ins_cluster_hist_392'] = 0.0
  result_map['schedule.ins_cluster_hist_393'] = 0.0
  result_map['schedule.ins_cluster_hist_394'] = 0.0
  result_map['schedule.ins_cluster_hist_395'] = 0.0
  result_map['schedule.ins_cluster_hist_396'] = 0.0
  result_map['schedule.ins_cluster_hist_397'] = 0.0
  result_map['schedule.ins_cluster_hist_398'] = 0.0
  result_map['schedule.ins_cluster_hist_399'] = 0.0
  result_map['schedule.ins_cluster_hist_400'] = 0.0

  # FOR PERFORMANCE MEASUREMENT
  result_map['commit.ipc'] = 0.0

# INSERT ELEMENTS IN THE RESULTS TABLE
def update_results_table (results_table, result_map):
  for result_param in result_map:
    result_val = result_map[result_param]
    results_table[result_param].append (result_val)

# UPDATE RESULT MAP FOR EACH INDIVIDUAL BENCHMARK
def update_result_map (result_param, result_value, result_map):
  if result_param in result_map:
    result_map[result_param] += float(result_value)

# READ THE SIMPOINTPOINT FILE AND ITS WEIGHTS
def parseSimpointFiles (in_paths, results_table, result_map):
  for in_path in in_paths:
    init_params (result_map) # RESET ALL VALUES

    # OPEN FILE
    try:
      pF_in = open (in_path, 'r')
      if os.path.getsize (in_path) == 0 or not os.path.isfile (in_path):
        print "File " + str (in_path) + " is empty. Continuing to the next file"
    except IOError:
      print '\tWARNING: Failed to open file ' + in_path
      update_results_table (results_table, result_map) # MISSING RESULT
      continue

    # COLLECT AND ANALYSE RESULTS
    tokens = []
    for line in pF_in.readlines ():
      tokens = line.strip ().split ()
      if len (tokens) > 2:
        result_param = tokens[1]
        result_value = 0
        if result_param.startswith ( 'bbWinBuf' ): # SOME POST-PROCESSING
          result_param = 'bbWinBuf' + result_param[11:]
        elif result_param.startswith ( 'bbWindow' ): # SOME POST-PROCESSING
          result_param = 'bbWindow' + result_param[11:]
        elif result_param.startswith ( 'lrfManager' ): # SOME POST-PROCESSING
          result_param = 'lrfManager' + result_param[13:]
        if result_param[-1] != ':':
          result_param = tokens[2]
          result_param = tokens[1]+'.'+result_param[:-1]
          result_value = tokens[3]
        else:
          result_param = result_param[:-1]
          result_value = tokens[2]
        update_result_map (result_param, result_value, result_map)
        results_verif.ins_cnt_verif (result_param, result_value, in_path)

    # AGGREGATE RESULTS
    update_results_table (results_table, result_map)

################
##### MAIN #####
if __name__ == "__main__":
  have_threshold=False
  thr1=-1.0
  thr2=-1.0
  thr2_mode='DY'
  if len (sys.argv) != 4:
    print "\n** This tool generates results tables for all SPECInt benchmarks **"
    print "USAGE: ./results_gen.py <MODE> <INPUT_DIR> <OUTPUT_DIR>"
    print "        [MODE: c?_s?_r?_m?]"
    sys.exit (0)
  else:
    mode = sys.argv[1]
    result_in_dir = sys.argv[2]
    result_out_dir = sys.argv[3]

  results_table = defaultdict(list)
  result_map = {}

  b_names = init_bench (results_table)
  in_paths = init_files (b_names, mode, result_in_dir)
  parseSimpointFiles (in_paths, results_table, result_map)
  print_headers (result_out_dir, results_table)
  print_results_table (result_out_dir, results_table, mode)

  print "*** DONE ***\n"
################
