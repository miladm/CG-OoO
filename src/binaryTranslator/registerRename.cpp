/*******************************************************************************
 *  registerRename.cpp
 ******************************************************************************/

#include "registerRename.h"
#include <map>

map<long int,long int> writeRegRenMap;
map<long int,long int> readRegRenMap;
long int nextRenReg = INIT_RENAME_REG_NUM;

long int renameWriteReg(long int reg) {
	nextRenReg++;
    Assert(reg < NUM_REGISTERS && "Invalid Register Number");
    Assert(nextRenReg > INIT_RENAME_REG_NUM && "Invalid Rename Register Number");
    writeRegRenMap.insert(pair<long int,long int>(reg,nextRenReg));
}

void renameReadReg(int indx, long int renReg) {
	Assert(_rt->Nth(indx) == READ);
    int num1 = _r->NumElements();
    if (renReg != -1) {
        _r->RemoveAt(indx);
        _r->InsertAt(renReg,indx);
    }
    int num2 = _r->NumElements();
    Assert (num1 == num2);
}

long int getRenamedReg(long int reg) {
	if (writeRegRenMap.count(reg) > 0)
        return writeRegRenMap.find(reg)->second;
    else
        return -1; //reg is not returned
}