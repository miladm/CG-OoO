/*******************************************************************************
 *  port_test.cpp
 ******************************************************************************/
#include "port_test.h"

using namespace std;

void port_test() {
    port<int> fromAtoB(10, 10);
    for (int i = 1; i < 150; i++) {
        int* ins = new int;
        *ins = i;

        if (i < 110) {
            int *gotIns = fromAtoB.getFront(i);
            if (gotIns == NULL)
            	cout << "GET: queue is empty" << endl;
            else
            	cout << "GET: got a new instruction (" << *gotIns << ") @ cycle " << i << endl;
        }
        
        if (i < 40 || i > 60) {
            if (fromAtoB.pushBack(ins, i) == FULL_BUFF)
                cout << "PUSH: buffer is full @ cycle " << i << endl;
            else
            	cout << "PUSH: stores an insi (" << i << ") in queue @ cycle " << i << endl;
        }
    }
}
