/*******************************************************************************
 *  logGen.cpp
 ******************************************************************************/

#include "logGen.h"

void writeToFile (List<basicblock*> *bbList, string *program_name, SCH_MODE sch_mode, REG_ALLOC_MODE reg_alloc_mode, CLUSTER_MODE cluster_mode, LENGTH cluster_size) {
    string reg_alloc_mode_s, sch_mode_s, cluster_mode_s;
    if (sch_mode == NO_LIST_SCH) sch_mode_s = "no_list_sch";
    else if (sch_mode == LIST_SCH) sch_mode_s = "list_sch";
    else Assert ("invalid scheduling mode");

    if (reg_alloc_mode == GLOBAL) reg_alloc_mode_s = "global_reg";
    else if (reg_alloc_mode == LOCAL_GLOBAL) reg_alloc_mode_s = "local_global_reg";
    else Assert ("invalid reg allocation mode");

    if (cluster_mode == BASICBLOCK) cluster_mode_s = "bb";
    else if (cluster_mode == SUPERBLOCK) cluster_mode_s = "sb";
    else if (cluster_mode == PHRASEBLOCK) cluster_mode_s = "pb";
    else Assert ("invalid scheduling mode");

	FILE* outFile;
    string out_dir = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/output_files/";
    char cluster_size_s [30]; sprintf (cluster_size_s, "%d", cluster_size);
    string out_file_path = out_dir + cluster_mode_s + "/" + reg_alloc_mode_s + "/" + sch_mode_s + "/" + (*program_name) + "_" + cluster_size_s + "_bbSiz_obj.s";
	if ((outFile  = fopen (out_file_path.c_str (), "w")) == NULL) 
		Assert ("Unable to open the output file.");
    cout << "OUTPUT .s FILE: " << out_file_path << endl;

	for (int i =  0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
		fprintf (outFile, "{,%lx\n", bb->getID ());
		if (bb->hasHeader ()) {
			fprintf (outFile, "H,%lx\n", bb->getBBbrHeader ());
		}
        int list_size = (sch_mode == LIST_SCH) ? bb->getBbSize_ListSch () : bb->getBbSize ();
		for (int j = 0; j < list_size; j++) {
			instruction* ins = (sch_mode == LIST_SCH) ? bb->getInsList_ListSchedule ()->Nth (j) : bb->getInsList ()->Nth (j);
			if (ins->getType () == 'M') {
				if (ins->isWrMemType ()) {
					fprintf (outFile, "W,%d,-memAddr-,%lx,%d,%s", ins->isUPLDdep (), ins->getInsAddr (), ins->getMemAccessSize (), ins->getArchRegisterStr ().c_str ());
				} else if (ins->isRdMemType ()) {
					fprintf (outFile, "R,%d,-memAddr-,%lx,%d,%s", ins->isUPLDdep (), ins->getInsAddr (), ins->getMemAccessSize (), ins->getArchRegisterStr ().c_str ());
				} else {
					Assert (true == false && "ERROR: A memory operation is either read or write");
				}
			} else if (ins->getType () == 'j' || ins->getType () == 'c' || ins->getType () == 'b' || ins->getType () == 'r') {
				fprintf (outFile, "%c,%d,%lx,-brTaken-,%lx,%s", ins->getType (), ins->isUPLDdep (), ins->getInsAddr (), ins->getInsDstAddr (), ins->getArchRegisterStr ().c_str ());
			} else if (ins->getType () == 'o' || ins->getType () == 'n' || ins->getType () == 's') { /*-- A, D, F, N for ins->getType () --*/
				fprintf (outFile, "%c,%d,%lx,%s", ins->getType (), ins->isUPLDdep (), ins->getInsAddr (), ins->getArchRegisterStr ().c_str ());
			} else {
				Assert (true == false && "Unrecognized instruction type");
			}
		}
		fprintf (outFile, "}\n");
	}
}
