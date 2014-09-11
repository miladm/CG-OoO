/*******************************************************************************
 * table.cpp
 *******************************************************************************/

#include "table.h"

/***********************************************/
/******************* TABLE *********************/
/***********************************************/
template <typename tableType_T>
table<tableType_T>::table (LENGTH len, 
                           TABLE_TYPE table_type, 
                           WIDTH rd_port_cnt, 
                           WIDTH wr_port_cnt,
                           sysClock* clk,
                           string table_name = "table") 
    : unit (table_name, clk),
      _wr_port (wr_port_cnt, WRITE, clk, table_name + ".wr_wire"),
      _rd_port (rd_port_cnt, READ,  clk, table_name + ".rd_wire"),
      _table_size (len),
      _table_type (table_type),
      s_table_empty_cyc (g_stats.newScalarStat (table_name, "empty_cyc", "Number of cycles with table empty", 0, NO_PRINT_ZERO)),
      s_table_full_cyc  (g_stats.newScalarStat (table_name, "full_cyc", "Number of cycles with table full", 0, NO_PRINT_ZERO)),
      s_table_size_rat  (g_stats.newRatioStat (clk, table_name, "size_rat", "Average table size", 0, NO_PRINT_ZERO))
{
	Assert (_table_size > 0 && 
            rd_port_cnt > 0 && rd_port_cnt <= _table_size &&
            wr_port_cnt > 0 && wr_port_cnt <= _table_size);
    _cycle = START_CYCLE;
}

template <typename tableType_T>
BUFF_STATE table<tableType_T>::pushBack (tableType_T obj) {
    Assert (_wr_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
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
    Assert (_wr_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
    Assert (getTableSize () > 0);
	tableType_T elem = table<tableType_T>::_table.Last ()->_element;
	delete table<tableType_T>::_table.Nth (getTableSize () - 1);
	table<tableType_T>::_table.RemoveAt (getTableSize () - 1);
	return elem;
}

template <typename tableType_T>
tableType_T table<tableType_T>::getBack () {
    Assert (_wr_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
    Assert (getTableSize () > 0);
	return table<tableType_T>::_table.Last ()->_element;
}

template<typename tableType_T>
BUFF_STATE table<tableType_T>::getTableState () {
    Assert (getTableSize () <= _table_size);
	if (getTableSize () == _table_size) {
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
        s_table_empty_cyc++;
    } else if (getTableState () == FULL_BUFF) {
        s_table_full_cyc++;
    }
    s_table_size_rat += getTableSize ();
}

template <typename tableType_T>
bool table<tableType_T>::hasFreeWire (AXES_TYPE axes_type) {
    if (axes_type == READ) {
        return _rd_port.hasFreeWire ();
    } else {
        return _wr_port.hasFreeWire ();
    }
}

template <typename tableType_T>
void table<tableType_T>::updateWireState (AXES_TYPE axes_type) {
    CYCLE now = _clk->now ();
    if (_cycle < now) {
        _wr_port.updateWireState ();
        _rd_port.updateWireState ();
        _cycle = now;
    } else if (_cycle == now) {
        if (axes_type == READ)
            _rd_port.updateWireState ();
        else
            _wr_port.updateWireState ();
    }
}

/***********************************************/
/***************** CAM TABLE *******************/
/***********************************************/
template <typename tableType_T>
CAMtable<tableType_T>::CAMtable (LENGTH len, 
                              WIDTH rd_port_cnt, 
                              WIDTH wr_port_cnt,
                              sysClock* clk,
                              string table_name = "CAMtable") 
    : table<tableType_T> (len, CAM_ARRAY,
                          wr_port_cnt, rd_port_cnt, 
                          clk, table_name)
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
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
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
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
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
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
     return table<tableType_T>::_table.Nth (0)->_element;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::getNth (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert (table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
     return table<tableType_T>::_table.Nth (indx)->_element;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::pullNth (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
     tableType_T elem = table<tableType_T>::_table.Nth(indx)->_element;
     table<tableType_T>::_table.RemoveAt (indx);
     return elem;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::getLast () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert (table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first"); //TODO - don't think this is needed, is it?
#endif
     return table<tableType_T>::_table.Last()->_element;
}

/***********************************************/
/***************** RAM TABLE *******************/
/***********************************************/
template <typename tableType_T>
RAMtable<tableType_T>::RAMtable (LENGTH len, 
                              WIDTH rd_port_cnt, 
                              WIDTH wr_port_cnt,
                              sysClock* clk,
                              string table_name = "RAMtable") 
    : table<tableType_T> (len, RAM_ARRAY,
                          wr_port_cnt, rd_port_cnt, 
                          clk, table_name)
{}

/***********************************************/
/***************** FIFO TABLE ******************/
/***********************************************/
template <typename tableType_T>
FIFOtable<tableType_T>::FIFOtable (LENGTH len, 
                              WIDTH rd_port_cnt, 
                              WIDTH wr_port_cnt,
                              sysClock* clk,
                              string table_name = "FIFOtable") 
    : table<tableType_T> (len, FIFO_ARRAY,
                          wr_port_cnt, rd_port_cnt, 
                          clk, table_name)
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
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
    return table<tableType_T>::_table.Nth (0)->_element;
}

template <typename tableType_T>
tableType_T FIFOtable<tableType_T>::popFront () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
    tableType_T dyIns = table<tableType_T>::_table.Nth (0)->_element;
    delete table<tableType_T>::_table.Nth (0);
    table<tableType_T>::_table.RemoveAt (0);
    return dyIns;
}


/** TEMPLATES **/

/* DYN INS */
template struct TableElement<dynInstruction*>;
template class table<dynInstruction*>;
template class CAMtable<dynInstruction*>;
template class RAMtable<dynInstruction*>;
template class FIFOtable<dynInstruction*>;

/* BB INS */
template struct TableElement<bbInstruction*>;
template class table<bbInstruction*>;
template class CAMtable<bbInstruction*>;
template class RAMtable<bbInstruction*>;
template class FIFOtable<bbInstruction*>;

/* DYN BB */
template struct TableElement<dynBasicblock*>;
template class table<dynBasicblock*>;
template class CAMtable<dynBasicblock*>;
