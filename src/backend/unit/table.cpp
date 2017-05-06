/*******************************************************************************
 * table.cpp
 *******************************************************************************/

#include "table.h"

/***********************************************/
/******************* TABLE *********************/
/***********************************************/
template <typename tableType_T>
table<tableType_T>::table (TABLE_TYPE table_type, sysClock* clk,
                           const YAML::Node& root,
                           string table_name = "table")
    : unit (table_name, clk),
      _wr_port (WRITE, clk, root, table_name + ".wr_wire"),
      _rd_port (READ,  clk, root, table_name + ".rd_wire"),
      _table_type (table_type),
      _e_table (table_name, root),
      s_table_empty_cyc (g_stats.newScalarStat (table_name, "empty_cyc", "Number of cycles with table empty", 0, NO_PRINT_ZERO)),
      s_table_full_cyc  (g_stats.newScalarStat (table_name, "full_cyc", "Number of cycles with table full", 0, NO_PRINT_ZERO)),
      s_table_size_rat  (g_stats.newRatioStat (clk->getStatObj (), table_name, "size_rat", "Average table size", 0, NO_PRINT_ZERO))
{
    WIDTH rd_wire_cnt, wr_wire_cnt;
    root["rd_wire_cnt"] >> rd_wire_cnt;
    root["wr_wire_cnt"] >> wr_wire_cnt; 
    root["size"] >> _table_size;

	Assert (_table_size > 0 && 
            rd_wire_cnt > 0 && rd_wire_cnt <= _table_size &&
            wr_wire_cnt > 0 && wr_wire_cnt <= _table_size);
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
    _e_table.ramAccess ();
	return AVAILABLE_BUFF;
}

template <typename tableType_T>
tableType_T table<tableType_T>::popBack () {
    Assert (_wr_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
    Assert (getTableSize () > 0);
	tableType_T elem = table<tableType_T>::_table.Last ()->_element;
	delete table<tableType_T>::_table.Nth (getTableSize () - 1);
	table<tableType_T>::_table.RemoveAt (getTableSize () - 1);
    _e_table.ramAccess ();
	return elem;
}

template <typename tableType_T>
tableType_T table<tableType_T>::getBack () {
    Assert (_wr_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
    Assert (getTableSize () > 0);
    _e_table.ramAccess ();
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
void table<tableType_T>::updateWireState (AXES_TYPE axes_type, list<string> wire_name, bool update_wire) {
    CYCLE now = _clk->now ();
    if (_cycle < now) {
        _wr_port.updateWireState (wire_name, axes_type == WRITE ? update_wire : false);
        _rd_port.updateWireState (wire_name, axes_type == READ ? update_wire : false);
        _cycle = now;
    } else if (_cycle == now) {
        if (axes_type == READ)
            _rd_port.updateWireState (wire_name, update_wire);
        else
            _wr_port.updateWireState (wire_name, update_wire);
    }
}

template <typename tableType_T>
void table<tableType_T>::camAccess (SCALAR num_access) {
    table<tableType_T>::_e_table.camAccess (num_access);
}

template <typename tableType_T>
void table<tableType_T>::cam2Access (SCALAR num_access) {
    table<tableType_T>::_e_table.cam2Access (num_access);
}

template <typename tableType_T>
void table<tableType_T>::ramAccess (SCALAR num_access) {
    table<tableType_T>::_e_table.ramAccess (num_access);
}

template <typename tableType_T>
void table<tableType_T>::ram2Access (SCALAR num_access) {
    table<tableType_T>::_e_table.ram2Access (num_access);
}

template <typename tableType_T>
void table<tableType_T>::fifoAccess (SCALAR num_access) {
    table<tableType_T>::_e_table.fifoAccess (num_access);
}

/***********************************************/
/***************** CAM TABLE *******************/
/***********************************************/
template <typename tableType_T>
CAMtable<tableType_T>::CAMtable (sysClock* clk,
                                 const YAML::Node& root,
                                 string table_name = "CAMtable")
    : table<tableType_T> (CAM_ARRAY, clk, root, table_name)
{}

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
    table<tableType_T>::_e_table.ramAccess ();
    return dyIns;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::pullNextReady (LENGTH next_ins_indx) {
    tableType_T obj = table<tableType_T>::_table.Nth (next_ins_indx)->_element;
    delete table<tableType_T>::_table.Nth (next_ins_indx);
    table<tableType_T>::_table.RemoveAt (next_ins_indx);
    table<tableType_T>::_e_table.ramAccess ();
    return obj;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::pullNth (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
    tableType_T elem = table<tableType_T>::_table.Nth(indx)->_element;
    delete table<tableType_T>::_table.Nth (indx);
    table<tableType_T>::_table.RemoveAt (indx);
    table<tableType_T>::_e_table.camAccess ();
    return elem;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::getFront () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert ( table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
    table<tableType_T>::_e_table.ramAccess ();
    return table<tableType_T>::_table.Nth (0)->_element;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::getNth (LENGTH indx) {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    Assert (table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first");
#endif
    table<tableType_T>::_e_table.camAccess ();
    return table<tableType_T>::_table.Nth (indx)->_element;
}

template <typename tableType_T>
tableType_T CAMtable<tableType_T>::getLast () {
#ifdef ASSERTION
    Assert (table<tableType_T>::_table.NumElements () > 0);
    //Assert (table<tableType_T>::_rd_port.getNumFreeWires () > 0 && "must have checked the available ports count first"); //TODO - don't think this is needed, is it?
#endif
    table<tableType_T>::_e_table.ramAccess ();
    return table<tableType_T>::_table.Last()->_element;
}

/***********************************************/
/***************** RAM TABLE *******************/
/***********************************************/
template <typename tableType_T>
RAMtable<tableType_T>::RAMtable (sysClock* clk,
                                 const YAML::Node& root,
                                 string table_name = "RAMtable")
    : table<tableType_T> (RAM_ARRAY, clk, root, table_name)
{}

/***********************************************/
/***************** FIFO TABLE ******************/
/***********************************************/
template <typename tableType_T>
FIFOtable<tableType_T>::FIFOtable (sysClock* clk,
                                   const YAML::Node& root,
                                   string table_name = "FIFOtable") 
    : table<tableType_T> (FIFO_ARRAY, clk, root, table_name)
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
    table<tableType_T>::_e_table.fifoAccess ();
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
    table<tableType_T>::_e_table.fifoAccess ();
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
