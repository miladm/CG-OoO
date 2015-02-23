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
  result_map['Energy'] = 0.0
  result_map['EU.e_eu'] = 0.0
  result_map['stBuff.e_ram'] = 0.0
  result_map['iWindow.e_ram'] = 0.0
  result_map['iWindow.rd_wire.e_wire'] = 0.0
  result_map['iWindow.wr_wire.e_wire'] = 0.0
  result_map['iROB.e_ram'] = 0.0
  result_map['iROB.rd_wire.e_wire'] = 0.0
  result_map['iROB.wr_wire.e_wire'] = 0.0
  result_map['rfManager.e_ram'] = 0.0
  result_map['registerFile.rd_wire.e_wire'] = 0.0
  result_map['registerFile.wr_wire.e_wire'] = 0.0
  result_map['BTB.e_cam'] = 0.0
  result_map['2bc_gskew.e_ram'] = 0.0
  result_map['registerRename.wr_wire.e_wire'] = 0.0
  result_map['registerRename.rd_wire.e_wire'] = 0.0
  result_map['rfManager.rf.e_ram'] = 0.0
  result_map['rfManager.rat.e_ram'] = 0.0
  result_map['rfManager.apr.e_ram'] = 0.0
  result_map['rfManager.arst.e_ram'] = 0.0
  result_map['SQ.wr_wire.e_wire'] = 0.0
  result_map['SQ.rd_wire.e_wire'] = 0.0
  result_map['SQ.e_cam'] = 0.0
  result_map['SQ.e_ram'] = 0.0
  result_map['LQ.wr_wire.e_wire'] = 0.0
  result_map['LQ.rd_wire.e_wire'] = 0.0
  result_map['LQ.e_cam'] = 0.0
  result_map['LQ.e_ram'] = 0.0
  result_map['dtlb.e_cam'] = 0.0
  result_map['iROB.wr_wire.e_wire'] = 0.0
  result_map['iROB.rd_wire.e_wire'] = 0.0
  result_map['iROB.e_ram'] = 0.0
  result_map['ResStn_0.wr_wire.e_wire'] = 0.0
  result_map['ResStn_0.rd_wire.e_wire'] = 0.0
  result_map['ResStn_0.e_cam'] = 0.0
  result_map['ResStn_0.e_cam2'] = 0.0
  result_map['ResStn_0.e_ram'] = 0.0
  result_map['GlobalRegisterRename.wr_wire.e_wire'] = 0.0
  result_map['GlobalRegisterRename.rd_wire.e_wire'] = 0.0
  result_map['grfManager.grf.e_ram'] = 0.0
  result_map['grfManager.grat.e_ram'] = 0.0
  result_map['grfManager.gapr.e_ram'] = 0.0
  result_map['grfManager.garst.e_ram'] = 0.0
  result_map['bbROB.wr_wire.e_wire'] = 0.0
  result_map['bbROB.rd_wire.e_wire'] = 0.0
  result_map['bbROB.e_ram'] = 0.0
  result_map['bbWindow.wr_wire.e_wire'] = 0.0
  result_map['bbWindow.rd_wire.e_wire'] = 0.0
  result_map['bbWindow.e_cam'] = 0.0
  result_map['bbWindow.e_ram'] = 0.0
  result_map['lrfManager.e_ram'] = 0.0
  result_map['LocalRegisterFile.wr_wire.e_wire'] = 0.0
  result_map['LocalRegisterFile.rd_wire.e_wire'] = 0.0

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
        if result_param.startswith ( 'bbWindow' ): # SOME POST-PROCESSING
          result_param = 'bbWindow' + result_param[11:]
        elif result_param.startswith ( 'lrfManager' ): # SOME POST-PROCESSING
          result_param = 'lrfManager' + result_param[13:]
          print result_param
        if result_param[-1] != ':':
          result_param = tokens[2]
          result_param = result_param[:-1]
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
