/*******************************************************************************
 * table.cpp
 *******************************************************************************/

#include "table.h"

/***********************************************/
/******************* TABLE *********************/
/***********************************************/
template <typename tableType_T>
table<tableType_T>::table (LENGTH len = 1, 
                           TABLE_TYPE table_type = RAM_ARRAY, 
                           WIDTH rd_port_cnt = 1, 
                           WIDTH wr_port_cnt = 1,
                           string table_name = "table") 
    : unit (table_name),
      //s_table_empty_cyc (g_stats.newScalarStat (table_name, "table_empty_cyc", "Number of cycles with table empty", 0, NO_PRINT_ZERO)),
      //s_table_full_cyc  (g_stats.newScalarStat (table_name, "table_full_cyc", "Number of cycles with table full", 0, NO_PRINT_ZERO)),
      _table_size (len),
      _table_type (table_type),
      _wr_port_cnt (wr_port_cnt),
      _rd_port_cnt (rd_port_cnt)
{
	Assert (_table_size > 0 && 
            _rd_port_cnt > 0 && _rd_port_cnt <= _table_size &&
            _wr_port_cnt > 0 && _wr_port_cnt <= _table_size);
    _cycle = START_CYCLE;
    _num_free_wr_port = _wr_port_cnt;
    _num_free_rd_port = _rd_port_cnt;
}

template <typename tableType_T>
BUFF_STATE table<tableType_T>::pushBack (tableType_T obj) {
    Assert ( _num_free_wr_port > 0 && "must have checked the available ports count first");
	if (getTableSize () >= _table_size) {
        dbg.print (DBG_PORT, "%s: %s (cyc: )\n", _c_name.c_str (), "is FULL");
        return FULL_BUFF;
    }
	TableElement<tableType_T>* newEntry = new TableElement<tableType_T> (obj);
	table<tableType_T>::_table.Append (newEntry);
	return AVAILABLE_BUFF;
}

template <typename tableType_T>
tableType_T table<tableType_T>::popBack () {
    Assert (_num_free_wr_port > 0 && "must have checked the available ports count first");
    Assert (getTableSize () > 0);
	tableType_T elem = table<tableType_T>::_table.Last ()->_element;
	delete table<tableType_T>::_table.Nth (getTableSize () - 1);
	table<tableType_T>::_table.RemoveAt (getTableSize () - 1);
	return elem;
}

template <typename tableType_T>
tableType_T table<tableType_T>::getBack () {
    Assert (_num_free_wr_port > 0 && "must have checked the available ports count first");
    Assert (getTableSize () > 0);
	return table<tableType_T>::_table.Last ()->_element;
}

template <typename tableType_T>
bool table<tableType_T>::hasFreeRdPort (CYCLE now) {
    if (_cycle < now) {
        _num_free_rd_port = _rd_port_cnt;
        _num_free_wr_port = _wr_port_cnt;
        _cycle = now;
        return true;
    } else if (_cycle == now) {
        table<tableType_T>::_num_free_rd_port--;
        if (table<tableType_T>::_num_free_rd_port > 0) return true;
        else return false;
    }
    Assert (true == false && "should not have gotten here");
    return false;
}

template <typename tableType_T>
bool table<tableType_T>::hasFreeWrPort (CYCLE now) {
    if (_cycle < now) {
        _num_free_wr_port = _wr_port_cnt;
        _num_free_rd_port = _rd_port_cnt;
        _cycle = now;
        return true;
    } else if (_cycle == now) {
        _num_free_wr_port--;
        if (_num_free_wr_port > 0) return true;
        else return false;
    }
    Assert (true == false && "should not have gotten here");
    return false;
}

template<typename tableType_T>
BUFF_STATE table<tableType_T>::getTableState () {
	if (getTableSize () >= _table_size) {
        dbg.print (DBG_PORT, "%s: %s (cyc: )\n", _c_name.c_str (), "is FULL");
        return FULL_BUFF;
    } else if (getTableSize () == 0) {
        dbg.print (DBG_PORT, "%s: %s (cyc: )\n", _c_name.c_str (), "is EMPTY");
        return EMPTY_BUFF;
    } else {return AVAILABLE_BUFF;}
}

template <typename tableType_T>
LENGTH table<tableType_T>::getTableSize () {
    return table<tableType_T>::_table.NumElements ();
}

template <typename tableType_T>
tableType_T table<tableType_T>::getNth_unsafe (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
#endif
     return table<tableType_T>::_table.Nth (indx)->_element;
}

template <typename tableType_T>
void table<tableType_T>::removeNth_unsafe (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
#endif
    delete table<tableType_T>::_table.Nth (indx);
    table<tableType_T>::_table.RemoveAt (indx);
}

template <typename tableType_T>
void table<tableType_T>::regStat () {
    if (getTableState () == EMPTY_BUFF) {
        //s_table_empty_cyc++;
    } else if (getTableState () == FULL_BUFF) {
        //s_table_full_cyc++;
    }
}

/***********************************************/
/***************** CAM TABLE *******************/
/***********************************************/
template <typename tableType_T>
CAMtable<tableType_T>::CAMtable (LENGTH len = 1, 
                              WIDTH rd_port_cnt = 1, 
                              WIDTH wr_port_cnt = 1,
                              string table_name = "CAMtable") 
    : table<tableType_T> (len, CAM_ARRAY,
                          wr_port_cnt, rd_port_cnt, table_name)
{}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::pullNextReady (LENGTH next_ins_indx) {
    tableType_T obj = table<tableType_T>::_table.Nth (next_ins_indx)->_element;
    delete table<tableType_T>::_table.Nth (next_ins_indx);
    table<tableType_T>::_table.RemoveAt (next_ins_indx);
    return obj;
}

/*
template <typename tableType_T>
LENGTH CAMtable<tableType_T>::getNextReadyIndx () {
#ifdef ASSRTION
    Assert ( table<tableType_T>::_num_free_rd_port > 0 && "must have checked the available ports count first");
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert (_table_type == CAM_ARRAY);
#endif
    for (LENGTH i = 0; i < (LENGTH)table<tableType_T>::_table.NumElements (); i++) {
        tableType_T obj = table<tableType_T>::_table.Nth (i)._element;
        if (obj->isReady ()) {
            return i;
        }
    }
    return -1; // Didn't find 
}
*/

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::popFront () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_num_free_rd_port > 0 && "must have checked the available ports count first");
#endif
    tableType_T dyIns = table<tableType_T>::_table.Nth (0)->_element;
    delete table<tableType_T>::_table.Nth (0);
    table<tableType_T>::_table.RemoveAt (0);
    return dyIns;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::getFront () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_num_free_rd_port > 0 && "must have checked the available ports count first");
#endif
     return table<tableType_T>::_table.Nth (0)->_element;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::getNth (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert (table<tableType_T>::_num_free_rd_port > 0 && "must have checked the available ports count first");
#endif
     return table<tableType_T>::_table.Nth (indx)->_element;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::pullNth (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_num_free_rd_port > 0 && "must have checked the available ports count first");
#endif
     tableType_T elem = table<tableType_T>::_table.Nth(indx)->_element;
     table<tableType_T>::_table.RemoveAt (indx);
     return elem;
}

/***********************************************/
/***************** RAM TABLE *******************/
/***********************************************/
template <typename tableType_T>
RAMtable<tableType_T>::RAMtable (LENGTH len = 1, 
                              WIDTH rd_port_cnt = 1, 
                              WIDTH wr_port_cnt = 1,
                              string table_name = "RAMtable") 
    : table<tableType_T> (len, RAM_ARRAY,
                          wr_port_cnt, rd_port_cnt, table_name)
{}

/***********************************************/
/***************** FIFO TABLE ******************/
/***********************************************/
template <typename tableType_T>
FIFOtable<tableType_T>::FIFOtable (LENGTH len = 1, 
                              WIDTH rd_port_cnt = 1, 
                              WIDTH wr_port_cnt = 1,
                              string table_name = "FIFOtable") 
    : table<tableType_T> (len, FIFO_ARRAY,
                          wr_port_cnt, rd_port_cnt, table_name)
{}
/*
template <typename tableType_T>
bool FIFOtable<tableType_T>::isFrontReady () {
    Assert (table<tableType_T>::_table.NumElements () > 0);
    return table<tableType_T>::_table.Nth (0)._element->isReady () ? true : false;
}
*/
template <typename tableType_T>
tableType_T FIFOtable<tableType_T>::getFront () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_num_free_rd_port > 0 && "must have checked the available ports count first");
#endif
    return table<tableType_T>::_table.Nth (0)->_element;
}

template <typename tableType_T>
tableType_T FIFOtable<tableType_T>::popFront () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_num_free_rd_port > 0 && "must have checked the available ports count first");
#endif
    tableType_T dyIns = table<tableType_T>::_table.Nth (0)->_element;
    delete table<tableType_T>::_table.Nth (0);
    table<tableType_T>::_table.RemoveAt (0);
    return dyIns;
}


/** TEMPLATES **/
template struct TableElement<dynInstruction*>;
template class table<dynInstruction*>;
template class CAMtable<dynInstruction*>;
template class RAMtable<dynInstruction*>;
template class FIFOtable<dynInstruction*>;
