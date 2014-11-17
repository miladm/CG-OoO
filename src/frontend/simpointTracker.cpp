/*******************************************************************************
 * simpointTrack.cpp
 *******************************************************************************/

#include "simpointTracker.h"

/* GLOBAL VARIABLES */
static SIMP prev_ins_cnt = 0;
static SIMP thr_ins_cnt = 2 * BILLION;

/*--
 * FIND SIMPOINTS, ENABLE SIMULATION FLAGS, 
 * TERMINATE SIMPOINTS, DISABLE SIMULATION FLAGS
 --*/
inline BOOL simpointMode () 
{
	static SIMP simp_count = 0; 
    static bool finished_last_simpoint = (g_cfg->_simpoint.size () == 0) ? true : false;
	static SIMP next_simp = g_cfg->_simpoint.begin()->first; 
    static SIMP simp_ins_cnt_with_wp = 0;
    static clock_t simp_strt = 0;

    simp_ins_cnt_with_wp++;
	if (!g_var.g_wrong_path) {
		if (g_var.g_inSimpoint) { /* INSIDE SIMPOINT */
			g_var.g_simpInsCnt++;
			if (g_var.g_simpInsCnt >= SIMP_WINDOW_SIZE) { /* LEAVE SIMPOINT */
                clock_t simp_stop = double (clock ()) / CLOCKS_PER_SEC;
                g_stats.dumpSummary ();
	            g_msg.simEvent ("SIMPOINT SPEED: %f Ops/Sec\n", ((double)simp_ins_cnt_with_wp / (double)(simp_stop - simp_strt)));
	            g_msg.simEvent ("SIMPOINT END\n");
				g_var.g_inSimpoint = false;
				g_var.g_enable_wp = false;
				g_var.g_simpInsCnt = 0;
				PIN_RemoveInstrumentation ();
				g_var.g_enable_instrumentation = false;
                g_var.g_enable_bkEnd = false;
                g_cfg->_simpoint.erase (next_simp);
				next_simp = g_cfg->_simpoint.begin()->first;
                if (g_cfg->_simpoint.size () == 0) {
                    finished_last_simpoint = true;
	                g_msg.simStep ("DONE WITH LAST SIMPOINT");
                } else { finished_last_simpoint = false; }
			}
		} else { /* OUTSIDE SIMPOINT */
			if (g_var.g_insCountRightPath >= next_simp) { /* ENTER SIMPOINT */
	            g_msg.simEvent ("\nIN SIMPOINT #%d: %lu\n", ++simp_count, g_var.g_insCountRightPath);
				g_var.g_inSimpoint = true;
				g_var.g_enable_wp = true;
				g_var.g_simpInsCnt = 0;
                simp_ins_cnt_with_wp = 0;
				PIN_RemoveInstrumentation ();
				g_var.g_enable_instrumentation = true;
                g_var.g_enable_bkEnd = true;
                simp_strt = double (clock ()) / CLOCKS_PER_SEC;
            }
		}
    }

	return finished_last_simpoint;
}

/*-- COUNTS THE NUMBER OF DYNAMIC INSTRUCTIONS (WRONG-PATH INSTRUCTIONS INCLUDED) --*/
BOOL doCount (ScalarStat& s_pin_ins_cnt, ScalarStat& s_pin_trace_cnt, 
              ScalarStat& s_pin_wp_cnt, ScalarStat& s_pin_sig_cnt, 
              ScalarStat& s_pin_flush_cnt, ScalarStat& s_pin_sig_recover_cnt, 
              UINT32 bb_ins_cnt)
{
    if (!g_var.g_wrong_path) g_var.g_insCountRightPath += bb_ins_cnt;

    static clock_t past = 0.0;
    static clock_t now = double (clock ()) / CLOCKS_PER_SEC;
    SIMP countQ    = (SIMP) s_pin_ins_cnt.getValue () / MILLION;
    SIMP countDiff = (SIMP) s_pin_ins_cnt.getValue () - prev_ins_cnt;

    bool finished_last_simpoint = false;
    if (g_var.g_enable_simpoint) 
        finished_last_simpoint = simpointMode ();

    if (countDiff > thr_ins_cnt) 
    {
        cout << countDiff << " " << thr_ins_cnt << endl;
        now = double (clock ()) / CLOCKS_PER_SEC;
        cout << countQ << " million passed at " << double (clock ()) / CLOCKS_PER_SEC << " seconds. (Diff Time: " << now-past << ")" << endl;
        cout << "  correct path ins count: " << g_var.g_insCountRightPath << " (fraction: " << double (g_var.g_insCountRightPath)/ s_pin_ins_cnt.getValue () << ")" << endl;
        cout << "  wrong path ins count: " << g_var.g_total_wrong_path_count << endl;
        cout << "  wrong path count: " << s_pin_wp_cnt.getValue () << endl;
        cout << "  trace count: " << s_pin_trace_cnt.getValue () << endl;
        cout << "  app signal count: " << s_pin_sig_cnt.getValue () << endl;
        cout << "  pin signal count: " << s_pin_sig_cnt.getValue () << endl;
        cout << "  out of mem count: " << s_pin_flush_cnt.getValue () << endl;
        cout << "  recovery count: " << s_pin_sig_recover_cnt.getValue () << endl;
        cout << "  code cache flush count: " << s_pin_flush_cnt.getValue () << endl;
        cout << "  avg wrong-path length: " << double (s_pin_wp_cnt.getValue ()) / double (s_pin_wp_cnt.getValue ()) << endl;
        cout << "  code cache size (MB): " << double (g_var.g_codeCacheSize) / (1024.0 * 1024.0) << endl << endl;
        prev_ins_cnt = s_pin_ins_cnt.getValue ();
        past = now;
    }

    return finished_last_simpoint;
}


//LEGACY CODE
//
//static long unsigned imgInsCallCount_ = 0;
//static long unsigned imgInsMemCount_ = 0;
//VOID doImpCallCount_ (BOOL isCall)
//{
//    if (isCall)
//        imgInsCallCount_++;
//    unsigned long countRem = (unsigned long) s_pin_ins_cnt.getValue () % BILLION;
//    if (countRem == 0) {
//        cout << " (CALL_) :" << imgInsCallCount_ << endl;
//    }
//}
//
//VOID doImpMemCount_ (UINT32 isMem)
//{
//    if (isMem > 0)
//        imgInsMemCount_++;
//    unsigned long countRem = (unsigned long) s_pin_ins_cnt.getValue () % BILLION;
//    if (countRem == 0) {
//        cout << " (MEM_) :" << imgInsMemCount_ << endl;
//    }
//}
