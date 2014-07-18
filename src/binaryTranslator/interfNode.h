/*******************************************************************************
 *  interfNode.h
 ******************************************************************************/

#ifndef _INTERF_NODE_H
#define _INTERF_NODE_H

#include <set>
#include <algorithm>
#include "list.h"
#include "global.h"

class interfNode {
	public:
		interfNode(long int psudoReg);
		~interfNode();
		void assignReg(set<long int> &GRFset);
		void addEdge(interfNode* node);
		List<interfNode*>* getEdgeList();
		long int getPsudoReg();
		int getNeighborSize();
		void removeFromGraph();
		void removeAneighbor(interfNode* node);
		long int getReg();

	private:
		long int _psudoReg;
		long int _reg;
		List<interfNode*> *_edgeList_static; // Used for step 2 (graph coloring part)
		List<interfNode*> *_edgeList_dynamic;
		set<long int> _neighborSet;
};

#endif