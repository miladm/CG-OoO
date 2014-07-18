#!/usr/bin/python
from __future__ import division

import sys
import os.path
import gzip
import itertools
from glob import iglob
from collections import Counter
import marshal
import random
import linecache
import re
from datetime import datetime

ioPath = 'input_files/'
fileName = 'bzip2.s'
OpCodeDic = []
regDic    = []
opCodeLib = {}
inAddrLib = []
jmpDstLib = []
jmpOpLib  = []


def init_x86_lib():
  global OpCodeDic
  global regDic
  global jmpOpCodeDic
  x86OpCodeInput = 'lib/x86_opcode.txt'
  x86OpCodeFile  = open(x86OpCodeInput, 'r')
  for line in x86OpCodeFile:
    line = line.lstrip().rstrip()
    OpCodeDic.append(line)
  x86RegInput = 'lib/x86_registers.txt'
  x86RegFile  = open(x86RegInput, 'r')
  for line in x86RegFile:
    line = line.lstrip().rstrip()
    regDic.append(line)

def isAnOpCode(word):
  global OpCodeDic
  if word in OpCodeDic:
    return True
  else:
    return False

def isAregister(word):
  global regDic
  words = word.rstrip().lstrip().split(',')
  for i in range(0,len(words)):
    words[i] = words[i].rstrip().lstrip()
    if words[i] in regDic:
      return True
  else:
      return False

def findOp (line, insAddr):
  global jmpDstLib
  global jmpOpLib
  global opCodeLib
  words = line.lstrip().rstrip().split()
  # print '\n',words
  line = line.rstrip().lstrip()
  done = False
  for i in range(len(words)):
    words[i] = words[i].lstrip().rstrip()
    if len(words[i]) > 0:
      #matchObj = re.match(r'[\da-f]+.*', words[i], flags=0)
      matchObj = isAnOpCode(words[i])
      if matchObj:
        opCode = words[i].lstrip()
        opCodeLib[insAddr] = opCode
        if len(words) > i+1:
          matchObj1 = re.match(r'[\da-f]+', words[i+1], flags=0)
          matchObj2 = re.match(r'0x+', words[i+1], flags=0)
          matchObj3 = isAnOpCode(words[i+1])
          matchObj4 = isAregister(words[i+1])
          if matchObj1 and not matchObj2 and not matchObj3 and not matchObj4:
            brDst = words[i+1]
            if opCode == 'jmpq' or opCode == 'jmp':
	        # jump
              print 'j'
              print opCode
              print insAddr,',',brDst
              print line
            elif opCode == 'callq' or opCode == 'call':
	        # function call
              print 'c'
              print opCode
              print insAddr,',',brDst
              print line
            else:
	        # branch
              print 'b'
              print opCode
              print insAddr,',',brDst
              print line
            jmpDstLib.append((words[i],insAddr,words[i+1]))
            jmpDstLib.append(words[i+1])
            jmpOpLib.append(insAddr)
          else:
            print 'o'
            print opCode
            print insAddr
            print line
        elif opCode == 'ret' or opCode == 'retq' or opCode == 'iret' or opCode == 'iretd' or opCode == 'iretw':
		  print 'r'
		  print opCode
		  print insAddr
		  print line
        else:
          print 'o'
          print opCode
          print insAddr
          print line
        break #Avoid replicating an instruction many times in case of no match

        # done = True
        # break
  #if done == False:
  #  print line

def cfg():
  ## PASS 1
  assemblyInput = ioPath+fileName
  assemblyFile  = open(assemblyInput, 'r')        
  for line in assemblyFile:
    words = line.lstrip().rstrip().split()
    if len(words) > 0:
      matchObj = re.match(r'[\da-f]+:', words[0], flags=0)
      if matchObj:
        # print words[0]
        insAddr = words[0]
        insAddr = insAddr[:-1]
        inAddrLib.append(insAddr)
        findOp(line, insAddr)
  assemblyFile.close()
  ## PASS 2
  #global jmpDstLib
  #global jmpOpLib
  #global opCodeLib
  #assemblyInput = ioPath+fileName
  #assemblyFile  = open(assemblyInput, 'r')        
  #for line in assemblyFile:
  #  words = line.lstrip().rstrip().split()
  #  if len(words) > 0:
  #    matchObj = re.match(r'[\da-f]+:', words[0], flags=0)
  #    if matchObj:
  #      #print words[0]
  #      insAddr = words[0]
  #      insAddr = insAddr[:-1]
  #      if insAddr in jmpOpLib:
  #        print line
  #        print '-end\n'
  #      elif insAddr in opCodeLib and opCodeLib[insAddr] == 'retq':
  #        print line
  #        print '----\n'
  #      elif insAddr in jmpDstLib:
  #        print '-start\n'
  #        print line
  #      else:
  #        print line

def trace():
  insAddrMap = []
  ## PHRASE 1
  print 'Phase 1'
  assemblyInput = ioPath+fileName
  assemblyFile  = open(assemblyInput, 'r')        
  for line in assemblyFile:
    words = line.lstrip().rstrip().split()
    if len(words) > 0:
      matchObj = re.match(r'[\da-f]+:', words[0], flags=0)
      if matchObj:
        #print words[0]
        insAddr = words[0]
        insAddr = insAddr[:-1] #already a hex, without 0x
        print ''.join(('0x',insAddr))
        inAddrLib.append(''.join(('0x',insAddr)))
  assemblyFile.close()
  ## PHRASE 2
  print 'Phase 2'
  traceInput = '/scratch/tracesim/cs343_bzip_sb.out'
  # traceInput = '/Volumes/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_frontend/pin/trace_static.out'
  traceFile  = open(traceInput, 'r')
  count = 0
  for line in traceFile:
    words = line.strip().split(',')
    if len(words) < 2:
      continue
    # if len(words) < 2:
    #   continue
    # if words[0] == 'P;':
    #   continue
    if words[0] == "R" or words[0] == "W":
      if hex(int(words[2])) not in insAddrMap:
        # print words[0],hex(int(words[2]))
        insAddrMap.append(hex(int(words[2]))) #conversion to hex adds 0x
    else:
      if hex(int(words[1])) not in insAddrMap:
        # print words[0],hex(int(words[1]))
        insAddrMap.append(hex(int(words[1]))) #conversion to hex adds 0x
    
  # for line in traceFile:
  #   count += 1
  #   if count == 100000:
  #     print 'beat',datetime.now()
  #     count = 0
  #   words = line.strip().split(',')
  #   if len(words) < 3:
  #     continue #avoid empty lines
  #   insType = words[0]
  #   insAddr = words[1]
  #   if insAddr not in insAddrMap:
  #     insAddrMap.append(insAddr)
  #   else:
  #     continue
  #   indx = 2
  #   memDestAddr = -1
  #   brDestAddr = -1
  #   taken_notTaken = -1
  #   # print insType
  #   if insType == 'R' or insType == 'W':
  #     memDestAddr = words[indx]
  #     indx = 3
  #   elif insType == 'B':
  #     taken_notTaken = words[indx]
  #     brDestAddr = words[indx+1]
  #     indx = 4
  #   for elem in words[indx:-1]:
  #     temp = elem.strip().split('#')
  #     register = temp[0]
  #     read_write = temp[1]
  #     # print 'registers: ',register,read_write
  # traceFile.close()
  print 'DIFF'
  for item in inAddrLib:
    if item not in insAddrMap:
      print item
    

if __name__ == '__main__':
  #print'Command: ',sys.argv
  
  if len(sys.argv) < 3:
    sys.exit('\nUSAGE:\n \
              \tpython parse_x86.py <flag> <io_dir_path> [<ins_addr_file_path>]\n \
              \t<flag>: cfg | \n')

  # IO Directory Path
  ioPath = sys.argv[2]
  if len(sys.argv) == 4:
    fileName = sys.argv[3]

  #Run cfg generator
  if sys.argv[1] == 'cfg':
    #create a bra graph of a map
    init_x86_lib()
    cfg()
  elif sys.argv[1] == 'trace':
    init_x86_lib()
    trace()
  else:
    print 'ERROR: false usage\n'
    sys.exit(1)
