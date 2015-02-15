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
  TWENTY_MIL = 20000000
  if result_param == 'commit.ins_cnt' and float (result_val) < TWENTY_MIL:
    print '\tWARNING: SHORT SIMULATION (' + result_val + ' ins) - ' + in_path
