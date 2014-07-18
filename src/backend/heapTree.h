// File name: HeapTree.h
// Author: Arman Sahakyan
// Copyright (C) 2007, The CodeProject
// Contact: arman_sahakyan@edu.aua.am
#pragma once


template <class TID, class TDATA>
class CHeapTree
{
	int m_nSize;
	int m_nMAX;
	const int m_nInitMax;
	struct _NODE {
		TID id;
		TDATA data;
	};
	_NODE *m_data;

public:
	CHeapTree(int nInitMax = 100);
	~CHeapTree();

	bool IsEmpty() const { return m_nSize == 0; }
	int GetSize() const { return m_nSize; }

	void Insert(const TID &id, const TDATA &data);
	// Remove the element with the highest priority [if the heap is not empty]
	bool RemoveTop();
	bool RemoveAll();

	bool GetTopID(TID *pid) const {
		if (IsEmpty()) return false;
		*pid = m_data[0].id; 
		return true;
	}
	bool GetTopData(TDATA *pdata) const {
		if (IsEmpty()) return false;
		*pdata = m_data[0].data;
		return true;
	}
	bool GetData(const TID &id, TDATA *pdata) const;

	// Change the data of a node with TID = id and reformat the heap
	bool ResetData(const TID &id, const TDATA &data);

private:
	// Reconstructs the heap be starting the element with index = iRoot
	void _ReformatHeap(int iRoot);
};

