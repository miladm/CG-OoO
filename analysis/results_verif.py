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
def ins_cnt_verif (result_param, result_val, in_path):
  LO = 20000000
  HI = 35000000
  if result_param == 'commit.ins_cnt' and float (result_val) < LO:
    print '\tWARNING: SHORT COMMIT Simulation (' + result_val + ' ins) - ' + in_path
  elif result_param == 'commit.ins_cnt' and float (result_val) > HI:
    print '\tWARNING: LONG COMMIT Simulation (' + result_val + ' ins) - ' + in_path

  if result_param == 'pars.pin_ins_cnt' and float (result_val) < LO:
    print '\tWARNING: SHORT Pin Instrumentation (' + result_val + ' ins) - ' + in_path
  elif result_param == 'pars.pin_ins_cnt' and float (result_val) > HI:
    print '\tWARNING: LONG Pin Instrumentation (' + result_val + ' ins) - ' + in_path
