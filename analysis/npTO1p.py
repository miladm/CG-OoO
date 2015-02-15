#!/usr/bin/env python
from __future__ import division
from collections import deque
from time import time
import os, glob, os.path
import sys
import re
import os

#Gloval Variables
perf_map={}
perf_stat_map={}
base_path = "/scratch/milad/ondemandbp/milad_benchmark_runs_temp"
out_path = base_path+"/out"
cpi = 0.000
mode = ''
fileCount = 0

# Initialize the map between energy and performance paramters (McPAT vs. Gem5)
def init():
# For energy measurement
 perf_map['system.switch_cpus.fetch.Insts'] = 0
 perf_map['system.switch_cpus.commit.count'] = 0
 perf_map['system.switch_cpus.commit.int_insts'] = 0
 perf_map['system.switch_cpus.commit.branches'] = 0
 perf_map['system.switch_cpus.BPredUnit.lookups'] = 0
 perf_map['system.switch_cpus.BPredUnit.lookup_and_updates'] = 0
 perf_map['system.switch_cpus.iew.iewExecLoadInsts'] = 0
 perf_map['system.switch_cpus.iew.exec_stores'] = 0
 perf_map['system.switch_cpus.numCycles'] = 0
 perf_map['system.switch_cpus.idleCycles'] = 0
 perf_map['system.switch_cpus.rob.rob_reads'] = 0
 perf_map['system.switch_cpus.rob.rob_writes'] = 0
 perf_map['system.switch_cpus.rename.int_rename_lookups'] = 0
 perf_map['system.switch_cpus.rename.RenamedOperands'] = 0
 perf_map['system.switch_cpus.rename.fp_rename_lookups'] = 0
 perf_map['system.switch_cpus.iq.int_inst_queue_reads'] = 0
 perf_map['system.switch_cpus.iq.int_inst_queue_writes'] = 0
 perf_map['system.switch_cpus.iq.int_inst_queue_wakeup_accesses'] = 0
 perf_map['system.switch_cpus.iq.fp_inst_queue_reads'] = 0
 perf_map['system.switch_cpus.iq.fp_inst_queue_writes'] = 0
 perf_map['system.switch_cpus.iq.fp_inst_queue_wakeup_accesses'] = 0
 perf_map['system.switch_cpus.int_regfile_reads'] = 0
 perf_map['system.switch_cpus.fp_regfile_reads'] = 0
 perf_map['system.switch_cpus.int_regfile_writes'] = 0
 perf_map['system.switch_cpus.fp_regfile_writes'] = 0
 perf_map['system.switch_cpus.commit.function_calls'] = 0
 perf_map['system.switch_cpus.iq.int_alu_accesses'] = 0
 perf_map['system.switch_cpus.iq.fp_alu_accesses'] = 0
 perf_map['system.switch_cpus.commit.fp_insts'] = 0
 perf_map['system.cpu.icache.ReadReq_accesses'] = 0
 perf_map['system.cpu.icache.ReadReq_misses'] = 0
 perf_map['system.switch_cpus.BPredUnit.BTBLookups'] = 0
 perf_map['system.switch_cpus.BPredUnit.BTBHits'] = 0
 perf_map['system.switch_cpus.BPredUnit.usedRAS'] = 0
 perf_map['system.switch_cpus.BPredUnit.RASInCorrect'] = 0
 perf_map['system.cpu.dcache.ReadReq_accesses'] = 0
 perf_map['system.cpu.dcache.WriteReq_accesses'] = 0
 perf_map['system.cpu.dcache.ReadReq_misses'] = 0
 perf_map['system.cpu.dcache.WriteReq_misses'] = 0
 perf_map['system.l2.ReadReq_accesses'] = 0
#perf_map['system.l2.WriteReq_accesses'] = 0
 perf_map['system.l2.ReadReq_misses'] = 0
#perf_map['system.l2.WriteReq_misses'] = 0
 perf_map['system.switch_cpus.iew.branchMispredicts'] = 0
 perf_map['system.switch_cpus.iew.MisPred_usedBP'] = 0
 perf_map['system.switch_cpus.itb.fetch_accesses'] = 0
 perf_map['system.switch_cpus.itb.fetch_misses'] = 0
 perf_map['system.switch_cpus.itb.fetch_acv'] = 0
 perf_map['system.switch_cpus.dtb.data_accesses'] = 0
 perf_map['system.switch_cpus.dtb.data_misses'] = 0
 perf_map['system.switch_cpus.dtb.data_acv'] = 0
 perf_map['system.switch_cpus.iew.exec_refs'] = 0
# For Performance measurement
 perf_map['system.switch_cpus.cpi_total'] = 0.0
 perf_map['sim_ticks'] = 0
 perf_map['sim_insts'] = 0
 perf_map['system.switch_cpus.fetch.NB_access_BP'] = 0
 perf_map['system.switch_cpus.fetch.NB_access_BTB'] = 0
 perf_map['system.switch_cpus.BPredUnit.condPredicted'] = 0
 perf_map['system.switch_cpus.BPredUnit.condIncorrect'] = 0
 perf_map['system.switch_cpus.commit.branchMispredicts'] = 0
 perf_map['system.switch_cpus.fetch.hintlessInstCnt'] = 0
 perf_map['system.switch_cpus.fetch.totalInstCnt'] = 0
 perf_map['system.switch_cpus.fetch.totalInstCnt'] = 0
 perf_map['system.switch_cpus.commit.ins_used_BP'] = 0
 perf_map['system.switch_cpus.commit.ins_not_used_BP'] = 0
 perf_map['system.switch_cpus.iew.MisPred_usedBP'] = 0
 perf_map['system.switch_cpus.iew.MisPred_not_usedBP'] = 0
 perf_map['system.switch_cpus.fetch.UncondCtrl_misLabel'] = 0
 perf_map['system.switch_cpus.fetch.label_ST'] = 0
 perf_map['system.switch_cpus.fetch.label_SN'] = 0
 perf_map['system.switch_cpus.fetch.label_UN'] = 0
 perf_map['system.switch_cpus.fetch.label_UP'] = 0
 perf_map['system.switch_cpus.fetch.label_UN_DY_UP'] = 0
 perf_map['system.switch_cpus.fetch.label_DY'] = 0
 perf_map['system.switch_cpus.fetch.misLabel_ST'] = 0
 perf_map['system.switch_cpus.fetch.misLabel_SN'] = 0
 perf_map['system.switch_cpus.fetch.misLabel_DY'] = 0
 perf_map['system.switch_cpus.fetch.fetchedBranches'] = 0
 perf_map['system.switch_cpus.fetch.fetchedNonBranches'] = 0
 perf_map['system.switch_cpus.BPredUnit.DY_treated_SN'] = 0
 perf_map['system.switch_cpus.BPredUnit.ST_treated_SN'] = 0
 perf_map['system.switch_cpus.BPredUnit.DY_ST_treated_SN'] = 0
 perf_map['system.switch_cpus.commit.ins_used_GP'] = 0
 perf_map['system.switch_cpus.commit.ins_used_LP'] = 0


# Parse gem5 statistics file and create a map #
def parsePerfFile(pF_rd, weight):
  line_num=0
  for line in pF_rd.readlines():
    line_num+=1
    tokens = line.strip().split()
    if len(tokens) > 0 and line_num > 2 and tokens[1] != 'End':
      perfKey = tokens[0]
      perfValue = tokens[1]
      if perfKey in perf_map:
        if perfKey == 'system.switch_cpus.cpi_total':
          perf_map[perfKey] += float(perfValue)*float(weight)
        elif perfKey == 'sim_ticks':
          perf_map[perfKey] += int(perfValue)
        else:
          perf_map[perfKey] += int(perfValue)
  return line_num

# Write final PERF file with new input values #
def writeToPerfFile(pF_out):
  for key in perf_map:
    line = str(key)+"\t"+str(perf_map[key])+"\n"
    pF_out.write(line)

# Write final PERF file some addition computed values #
def writeToPerfFile_postStat(pF_out, mode):
  if mode == 'nodbp':
    accuracy = 1.0-perf_map["system.switch_cpus.iew.branchMispredicts"]/perf_map["system.switch_cpus.commit.count"]
    print "BP Accuracy (commit mispred / commit ins count): "+str(accuracy)
    pF_out.write("branch prediction accuracy"+'\t'+str(accuracy)+'\n')
  elif mode == 'odbp':
    accuracy_dy  = 1.0-perf_map["system.switch_cpus.iew.MisPred_usedBP"]/perf_map["system.switch_cpus.commit.ins_used_BP"]
    accuracy_st  = 1.0-perf_map["system.switch_cpus.iew.MisPred_not_usedBP"]/perf_map["system.switch_cpus.commit.ins_not_used_BP"]
    accuracy_tot = 1.0-(perf_map["system.switch_cpus.iew.MisPred_not_usedBP"]+perf_map["system.switch_cpus.iew.MisPred_usedBP"])/(perf_map["system.switch_cpus.commit.ins_not_used_BP"]+perf_map["system.switch_cpus.commit.ins_used_BP"])
    print "DY BP Accuracy (commit mispred / commit ins count): "+str(accuracy_dy)
    print "ST BP Accuracy (commit mispred / commit ins count): "+str(accuracy_st)
    print "TOTAL BP Accuracy (commit mispred / commit ins count): "+str(accuracy_tot)
    pF_out.write("DY branch prediction accuracy"+'\t'+str(accuracy_dy)+'\n')
    pF_out.write("ST branch prediction accuracy"+'\t'+str(accuracy_st)+'\n')
    pF_out.write("TOTAL branch prediction accuracy"+'\t'+str(accuracy_tot)+'\n')
  else:
    print 'ERROR: illegal mode'
    sys.exit(1)

# Read the simpointpoint file and its weights
def parseSimpointFiles(sF_in, wF_in, pF_out, b_name, thr1, thr2, thr2_mode, have_threshold, fileAffix, mode):
  weights = []
  indx = 0
  fileCount = 0
  for line in wF_in.readlines():
    tokens = line.strip().split()
    weights.append(float(tokens[0]))
  for line in sF_in.readlines():
    tokens = line.strip().split()
    simpoint = tokens[0]
    runFile = ''
    if mode == 'odbp':
      if have_threshold == False:
        runFile=out_path+"/"+b_name+"-baseline/"+simpoint+"00000000-100000000/odbp_runChkPtStat_"+b_name+"_"+fileAffix+".txt"
      else:
        runFile=out_path+"/"+b_name+"-baseline/"+simpoint+"00000000-100000000/odbp_runChkPtStat_"+b_name+"-"+thr1+"-"+thr2+"-"+thr2_mode+"_"+fileAffix+".txt"
    elif mode == 'nodbp':
      runFile=out_path+"/"+b_name+"-baseline/"+simpoint+"00000000-100000000/nodbp_runChkPtStat_"+b_name+"_"+fileAffix+".txt"
    else:
      sys.exit(0)
    try:
      perfFile_in = open(runFile, 'r')
      if os.path.getsize(runFile)==0 or not os.path.isfile(runFile):
        print "File "+str(runFile)+" is empty. Continuing to the next file"
        continue
    except IOError:
      print 'WARNING: Missing File: '+runFile
      print 'Skipping the file...'
      continue
    print runFile
    weight = weights[indx]
    if parsePerfFile(perfFile_in, weight) > 0:
      fileCount += 1
    indx += 1
  pF_out.write('number of files read = '+str(fileCount)+'\n')
  writeToPerfFile(pF_out)
  writeToPerfFile_postStat(pF_out, mode)

################
##### MAIN #####
if __name__ == "__main__":
  have_threshold=False
  thr1=-1.0
  thr2=-1.0
  thr2_mode='DY'
  if len(sys.argv) != 4 and len(sys.argv) != 7:
    print "\nUSAGE: ./npTO1p.py <benchmark_name> <mode> <fileAffix> [<thr1> <thr2> <thr2_mode>]\n"
    print "        --MODE"
    print "\t - odbp"
    print "\t - nodbp"
    print "        --SPEC CPU06 Benchmark Names:"
    print "\t - perlbench400"
    print "\t - bzip401"
    print "\t - gcc403"
    print "\t - mcf429"
    print "\t - gobmk445"
    print "\t - hmmer456"
    print "\t - sjeng458"
    print "\t - libquantum462"
    print "\t - h264ref464"
    print "\t - omnetpp471"
    print "\t - astar473"
    print "\t - xalanc483\n"
    sys.exit(0)
  else:
    b_name= sys.argv[1]
    mode= sys.argv[2]
    fileAffix= sys.argv[3]
    if len(sys.argv) == 7 and mode == 'odbp':
      have_threshold = True
      thr1=sys.argv[4]
      thr2=sys.argv[5]
      thr2_mode=sys.argv[6]
    elif len(sys.argv) == 4:
      dummy=0#do nothing here
    else:
      print "ERROR: Thresholds only apply to ODBP mode"
      sys.exit(0)
  sF_in  = "/simpoints_"+b_name
  wF_in  = "/weights_"+b_name
  if mode == 'odbp':
    if have_threshold == False:
      pF_out = out_path+"/"+b_name+"-baseline/odbp_stats_bl_"+b_name+"_"+fileAffix+".txt"
    else:
      pF_out = out_path+"/"+b_name+"-baseline/odbp_stats_bl_"+b_name+"-"+thr1+"-"+thr2+"-"+thr2_mode+"_"+fileAffix+".txt"
  elif mode == 'nodbp':
    pF_out = out_path+"/"+b_name+"-baseline/nodbp_stats_bl_"+b_name+"_"+thr2_mode+"_"+fileAffix+".txt"
  else:
    sys.exit(0)

  cpi = 0.0
  fileCount = 0
  simpoint_dir = base_path+"/simpoints"


  simpointFile_in = open(simpoint_dir + sF_in,  'r')
  weightFile_in   = open(simpoint_dir + wF_in,  'r')
  perfFile_out    = open(pF_out, 'w')

  init()
  parseSimpointFiles(simpointFile_in, weightFile_in, perfFile_out, b_name, thr1, thr2, thr2_mode, have_threshold, fileAffix, mode)

  simpointFile_in.close()
  weightFile_in.close()
  perfFile_out.close()

  print "\n*** Done... ***"
################
