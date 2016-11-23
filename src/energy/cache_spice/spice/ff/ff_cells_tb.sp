*************************
** INCLUDES *************
*************************
.param supply=1
.option scale=0.022u
.option runlvl=5
.option accurate post
.option dcic=0
.global vddgl
.option parhier=local
.option measform=1
.option list
.op
.protect
.lib '/afs/ir.stanford.edu/class/ee313/lib/opConditions.lib' TTTT
.unprotect

V_supply vddgl 0 dc=supply

************************
** PARAMETERS **********
************************
.include "parameters.sp"

.param thalf=1n

************************
** STIMULI *************
** A CLOCK PULSE TO 
** FLIP FF STATE FROM 
** 1 TO 0 ON A FLAT 
** ZERO INPUT
************************
Vclk clk 0 pwl(0.000n        0          'thalf'      0
+              'thalf+5p'    'supply'   '2*thalf+5p' 'supply'
+              '2*thalf+10p' 0          '5*thalf'    0)

Va j1 0 pwl(0.000n          0           '10p'     'supply'
+           '20p'           'supply'    '2*thalf' 'supply'
+           '2*thalf+10p'   0           '5*thalf' 0)

***********************
** NETLIST ************
***********************
.include "ff_cells.ckt"

**********************
** INITIAL CONDITION *
**********************
.IC v(xa.o1) = 0
.IC v(xa.o2) = 'supply'
.IC v(xa.o3) = 'supply'
.IC v(xa.o4) = 'supply'
.IC v(xa.o5) = 0
.IC v(xa.o6) = 'supply'

***********************
** ANALYSIS ***********
***********************
.tran 0.005ns 5.0ns sweep thalf 0.125n 5n 0.125n

.measure tran iavg
+ RMS    i(V_supply)  from='0.5*thalf'  to='2.5*thalf'

.measure tran ileak
+ RMS    i(V_supply)  from='4.5*thalf'  to='5.0*thalf'

.measure tran e_peracc
+ PARAM='(iavg-ileak)*supply*thalf*0.5'

.measure tran e_leak
+ PARAM='ileak*supply*thalf*0.5'

.END
