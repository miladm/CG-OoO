/*******************************************************************************
 *  stat.cpp
 ******************************************************************************/

#include "stat.h"

static string stat_path = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/stat";

void StatBBSizeStat(List<basicblock*> *bbList, string *program_name) {
	map<int,int> bbSizeDensity;
	int totalBBsize = 0;
	for (int i = 0; i < bbList->NumElements(); i++) {
		int size = bbList->Nth(i)->getBbSize();
		totalBBsize += size;
		if (bbSizeDensity.find(size) != bbSizeDensity.end())
			bbSizeDensity[size] += 1;
		else
			bbSizeDensity[size] = 1;
	}
	FILE *statOutputFile;
    string file_name = stat_path + "/" + (*program_name) + "_StatBBSizeCDF_stat.csv";
	if ((statOutputFile  = fopen(file_name.c_str(), "w")) == NULL)
		Assert("Unable to open the statistics output file.");
	float cdf = 0.0;
	for (map<int,int>::iterator it = bbSizeDensity.begin(); it != bbSizeDensity.end(); it++) {
		cdf += (float)it->second / (float)bbList->NumElements();
		fprintf(statOutputFile, "%d, %f\n", it->first, cdf);
	}
    printf ("StatBBSizeCDF: %s\n", file_name.c_str ());
}

void DynBBSizeStat(map<int,int> &bbSizeHist, string *program_name) {
	FILE *statOutputFile;
    string file_name = stat_path + "/" + (*program_name) + "_DynBBSizeHist_stat.csv";
	if ((statOutputFile  = fopen(file_name.c_str(), "w")) == NULL)
		Assert("Unable to open the statistics output file.");
	for (map<int,int>::iterator it = bbSizeHist.begin(); it != bbSizeHist.end(); it++) {
		fprintf(statOutputFile, "%d, %d\n", it->first, it->second);
	}
    printf ("DynBBSizeHist: %s\n", file_name.c_str ());
}

void StatNum_interBB_and_intra_BB_regs(List<basicblock*> *bbList, string *program_name) {
	FILE *statOutputFile;
	long int num_interBB_regs = 0, num_intraBB_regs = 0;
	for (int i = 0; i < bbList->NumElements(); i++) {
		basicblock* bb = bbList->Nth(i);
		List<instruction*>* insList = bb->getInsList();
		for (int j = 0; j < insList->NumElements(); j ++) {
			instruction *ins = insList->Nth(j);
			List<instruction*>* ancestors = ins->getRegAncestors();
			for (int k = 0; k < ancestors->NumElements(); k++) {
				instruction *anc = ancestors->Nth(k);
				if (bb->isInsAddrInBB(anc->getInsAddr())) {
					num_intraBB_regs++;
				} else {
					num_interBB_regs++;
				}
			}
		}
	}
	if ((statOutputFile  = fopen((stat_path + "/" + (*program_name) + "_inter_n_intra_BB_reg_stat.txt").c_str(), "w")) == NULL)
		Assert("Unable to open the statistics output file.");

	fprintf(statOutputFile, "Avg. Num INTRA-BB Regs: %f\n", (float)num_intraBB_regs/(float)bbList->NumElements());
	fprintf(statOutputFile, "Avg. Num INTER-BB Regs: %f\n", (float)num_interBB_regs/(float)bbList->NumElements());
}
