import subprocess
import sys

import spiceutils

def simulate_ff():
    devnull = open('junk', 'w')
    # REMOVE PREV SIMULATION OUTPUTS
    subprocess.call("rm -f ff_cells_tb.*", shell=True)
    # RUN HSPICE
    subprocess.call("hspice ../spice/ff/ff_cells_tb.sp", stdout=devnull,
                    stderr=subprocess.STDOUT, shell=True)
    devnull.close()

    stats = spiceutils.read_mt0('ff_cells_tb.mt0')

    for stat in stats:
        return stat
