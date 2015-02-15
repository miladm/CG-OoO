#!/usr/bin/env python
from __future__ import division
from collections import deque
from collections import defaultdict
from time import time
import os, glob, os.path
import sys
import re
import os

#GLOBAL VARIABLES
result_map={}

# INITIALIZE THE BENCHMARKS
def init_bench (results_table):
  b_names = ["400.perlbench", "401.bzip", "403.gcc", "429.mcf", \
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

# INITIALIZE THE MAP BETWEEN ENERGY AND PERFORMANCE PARAMTERS
def init_params ():
  # FOR ENERGY MEASUREMENT
  result_map['TOTAL'] = 0.0
  result_map['EU.e_eu'] = 0.0

  # FOR PERFORMANCE MEASUREMENT
  result_map['commit.ipc'] = 0.0

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

# INSERT ELEMENTS IN THE RESULTS TABLE
def update_results_table (results_table):
  for result_param in result_map:
    result_val = result_map[result_param]
    results_table[result_param].append (result_val)

# READ THE SIMPOINTPOINT FILE AND ITS WEIGHTS
def parseSimpointFiles (in_paths, results_table):
  for in_path in in_paths:
    init_params () # RESET ALL VALUES

    try:
      pF_in = open (in_path, 'r')
      if os.path.getsize (in_path) == 0 or not os.path.isfile (in_path):
        print "File " + str (in_path) + " is empty. Continuing to the next file"
    except IOError:
      print 'WARNING: Failed to open file ' + in_path
      update_results_table (results_table) # MISSING RESULT
      continue

    tokens = []
    for line in pF_in.readlines ():
      tokens = line.strip ().split ()
      if len (tokens) > 2:
        result_param = tokens[1]
        result_param = result_param[:-1]
        result_value = tokens[2]
        if result_param in result_map:
          result_map[result_param] += float(result_value)

    update_results_table (results_table)

################
##### MAIN #####
if __name__ == "__main__":
  have_threshold=False
  thr1=-1.0
  thr2=-1.0
  thr2_mode='DY'
  if len (sys.argv) != 4:
    print "\n** This tool generates results tables for all SPECInt Benchmarks **"
    print "USAGE: ./results_gen.py <MODE> <INPUT_DIR> <OUTPUT_DIR>"
    print "        [MODE: c?_s?_r?_m?]"
    sys.exit (0)
  else:
    mode = sys.argv[1]
    result_in_dir = sys.argv[2]
    result_out_dir = sys.argv[3]

  results_table = defaultdict(list)

  b_names = init_bench (results_table)
  in_paths = init_files (b_names, mode, result_in_dir)
  parseSimpointFiles (in_paths, results_table)
  print_headers (result_out_dir, results_table)
  print_results_table (result_out_dir, results_table, mode)

  print "\n*** DONE ***"
################
