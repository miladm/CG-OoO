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
  HI = 25000000
  if result_param == 'commit.ins_cnt' and float (result_val) < LO:
    print '\tWARNING: SHORT Simulation (' + result_val + ' ins) - ' + in_path
  elif result_param == 'commit.ins_cnt' and float (result_val) > HI:
    print '\tWARNING: LONG Simulation (' + result_val + ' ins) - ' + in_path
