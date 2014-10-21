#include <iostream>
#include "../base/types.hh"
#include "../base/intmath.hh"
#include "tournament.hh"
using namespace std;

int main() {
    TournamentBP *predictor;
    predictor = new TournamentBP(2048, 2, 2048, 11, 8192, 13, 2, 8192, 2, 0);
    for(int i = 0; i < 256; i++) {
        void *bp_hist = NULL;
        Addr addr = 0x21;
        bool prediction = predictor->lookup(addr, bp_hist);
        std::cout << hex << "0x" << addr << " " << prediction << endl;
        if(!prediction)
            predictor->update(addr, true, bp_hist, 1);
        else
            predictor->update(addr, true, bp_hist, 0);
    }
}
