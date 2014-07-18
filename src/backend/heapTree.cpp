// File name: HeapTree.cpp
// Author: Arman Sahakyan
// Copyright (C) 2007, The CodeProject
// Contact: arman_sahakyan@edu.aua.am
#include "heapTree.h"


template <class TID, class TDATA>
inline CHeapTree<TID, TDATA>::CHeapTree(int nInitMax) : m_nInitMax(nInitMax)
{
	m_nMAX = m_nInitMax;
	m_data = new _NODE[m_nMAX];
	m_nSize = 0;
}

template <class TID, class TDATA>
inline CHeapTree<TID, TDATA>::~CHeapTree()
{
	delete [] m_data;
}

template <class TID, class TDATA>
inline void CHeapTree<TID, TDATA>::Insert(const TID &id, const TDATA &data)
{
	if (m_nSize == m_nMAX - 1)
	{ // Resize the m_data array by doubling its size.
		m_nMAX += m_nMAX;
		_NODE *pTemp = m_data;
		m_data = new _NODE[m_nMAX];
		for (int j = 0; j < m_nSize; j ++)
			m_data[j] = pTemp[j];
		delete [] pTemp;
	}

	_NODE node = { id, data };
	m_data[m_nSize] = node;
	int iCur = m_nSize ++;
	int iParent = (iCur - 1) / 2;
	while (iParent >= 0 && m_data[iParent].data > m_data[iCur].data) {
		_NODE t = m_data[iCur];
		m_data[iCur] = m_data[iParent];
		m_data[iParent] = t;
		iCur = iParent;
		iParent = (iCur - 1) / 2;
	}
}

template <class TID, class TDATA>
inline bool CHeapTree<TID, TDATA>::RemoveTop()
{
	if (IsEmpty()) return false;

	if (m_nSize == m_nMAX / 2)
	{ // Resize the m_data array by shrinking its size twice.
		m_nMAX /= 2;
		_NODE *pTemp = m_data;
		m_data = new _NODE[m_nMAX];
		for (int j = 0; j < m_nSize; j ++)
			m_data[j] = pTemp[j];
		delete [] pTemp;
	}
	m_data[0] = m_data[-- m_nSize]; // move the last elem to the first place
	_ReformatHeap(0);
	return true;
}

template <class TID, class TDATA>
inline bool CHeapTree<TID, TDATA>::RemoveAll()
{
	if (IsEmpty()) return false;
	delete [] m_data;
	m_nMAX = m_nInitMax;
	m_data = new _NODE[m_nMAX];
	m_nSize = 0;
	return true;
}

template <class TID, class TDATA>
inline bool CHeapTree<TID, TDATA>::ResetData(const TID &id, const TDATA &dataNew)
{
	for (int j = 0; j < m_nSize; j ++)
		if (m_data[j].id == id) {
			if (m_data[j].data < dataNew) {
				m_data[j].data = dataNew;
				_ReformatHeap(0);
			}
			else {
				m_data[j].data = dataNew;
				_NODE node = m_data[j];
				int iCur = j;
				int iParent = (iCur - 1) / 2;
				while (iParent >= 0 && m_data[iParent].data > m_data[iCur].data) {
					_NODE t = m_data[iCur];
					m_data[iCur] = m_data[iParent];
					m_data[iParent] = t;
					iCur = iParent;
					iParent = (iCur - 1) / 2;
				}
			}
			return true;
		}
		return false; // id not found
}

template <class TID, class TDATA>
inline bool CHeapTree<TID, TDATA>::GetData(const TID &id, TDATA *pdata) const
{
	for (int j = 0; j < m_nSize; j ++)
		if (m_data[j].id == id) {
			*pdata = m_data[j].data;
			return true;
		}
	return false; // not found
}

template <class TID, class TDATA>
inline void CHeapTree<TID, TDATA>::_ReformatHeap(int iRoot)
{
	int iChild = 2 * iRoot + 1; // left child index
	if (iChild < m_nSize)
	{
		int iRightChild = iChild + 1; // right child index
		if (iRightChild < m_nSize)
		{
			if (m_data[iChild].data > m_data[iRightChild].data)
				iChild = iRightChild;
		}
		if (m_data[iChild].data < m_data[iRoot].data) {
			_NODE t = m_data[iRoot];
			m_data[iRoot] = m_data[iChild];
			m_data[iChild] = t;
			_ReformatHeap(iChild);
		}
	}
}
