#ifndef __CACHE_H__
#define __CACHE_H__

#include <cstdio>
#include <cstdlib>
#include <inttypes.h>
#include <cstring>
#include <cassert>
#include <optype.h>

#include <cacheline.h>

namespace Memory {

class NRU {
    private:
        int _ways;
        bool *_nrubits;
        // optimization so we dont have to keep
        // looping over the _nrubits array
        // every time
        int __setbits;
        void clearNRU() {
            memset(_nrubits, 0, _ways);
            __setbits = 0;
        }

        void setNRU(int way) {
            assert(__setbits < _ways);
            if(!_nrubits[way]) {
                __setbits++;
                _nrubits[way] = true;
            }
            if(__setbits == _ways) {
                clearNRU();
                _nrubits[way] = true;
                __setbits = 1;
            }
        }

    public:
         void init(int ways, int numsets, void *arg) {
            _ways = ways;
            _nrubits = new bool[_ways]();
            __setbits = 0;
        }

         void fini() {
            delete[] _nrubits;
        }

        ~NRU() {}

         void onHit(int way, uint64_t tag, void *arg) {
            setNRU(way);
        }

         int getRepl(uint64_t tag, CacheLine *currentTags, void *arg) {
            for(int i = 0; i < _ways; i++) {
                if(!_nrubits[i]) {
                    setNRU(i);
                    return i;
                }
            }
            assert(0);
            return -1;
        }

};

class RandomRepl {
    private:
        int _ways;
    public:
        void init(int ways, int numsets, void *arg) {
            _ways = ways;
        }

        void onHit(int way, uint64_t tag, void *arg) {
        }

        int getRepl(uint64_t tag, CacheLine *currentTags, void *arg) {
            return rand() % _ways;
        }

        void fini() {}
};

template<class REPL>
class Set {
    private:
        int _ways;
        CacheLine *_lines;
        REPL repl;
    public:
        Set() {}

        void init(int associativity, int numsets, void *arg)
        {
            _ways = associativity;
            _lines = new CacheLine[_ways]();
            repl.init(associativity, numsets, arg);
        }

        void fini() {
            repl.fini();
            delete[] _lines;
        }

        ~Set() {}

        CacheLine *peek(uint64_t tag) {
            for(int i = 0; i < _ways; i++) {
                if(_lines[i].tag == tag && _lines[i].state != LINE_INVALID) {
                    return &(_lines[i]);
                }
            }
            return NULL;
        }

        CacheLine *probe(uint64_t tag, void *arg) {
            for(int i = 0; i < _ways; i++) {
                if(_lines[i].tag == tag && _lines[i].state != LINE_INVALID) {
                    repl.onHit(i, tag, arg);
                    return &(_lines[i]);
                }
            }
            return NULL;
        }

        CacheLine *replace(uint64_t tag, void *arg) {
            CacheLine *line = probe(tag, arg);
            if(line)
                return line;
            int replWay = repl.getRepl(tag, _lines, arg);
            if(replWay == -1)
                return NULL;
            assert(replWay >= 0 && replWay < _ways);
            return &(_lines[replWay]);
        }

};

class DefaultIndex {
    public:
        static int getSet(uint64_t address, uint64_t _linesize, uint64_t _numsets) {
            assert(_numsets);
            uint64_t set = (address / _linesize) & (_numsets-1);
            assert(set < _numsets);
            return set;
        }
};

class BaseCache {
    private:
        uint64_t last_accessed_cycle;
    public:
        virtual uint64_t getTag(uint64_t address) =0;
        virtual CacheLine *peek(uint64_t address) =0;
        virtual CacheLine *probe(uint64_t address, void *arg) =0;
        virtual CacheLine *replace(uint64_t address, void *arg) =0;
        virtual int latency() =0;
        virtual bool get_port(OP_TYPE type) {
            assert(type != REPLY);
            if(last_accessed_cycle == sim_cycle) {
                return false;
            } else {
                last_accessed_cycle = sim_cycle;
                return true;
            }
        }
};

template<class REPL, class INDEX>
class GeneralCache : public BaseCache {
    private:
        Set<REPL> *_sets;
        int _numways;
        uint64_t _numsets;
        uint64_t _linesize;
        int _latency;

    public:
        GeneralCache() {
            _sets = NULL;
        }

        void init(int size, int assoc, int linesize, int lat, void *arg = NULL) {
            _numways = assoc;
            _numsets = size/assoc/linesize;
            _linesize = linesize;
            _latency = lat;
            _sets = new Set<REPL>[_numsets];
            for(int i = 0; i < _numsets; i++) {
                _sets[i].init(assoc, _numsets, arg);
            }
        }

        GeneralCache(int size, int assoc, int linesize, int lat, void *arg = NULL)
        {
            init(size, assoc, linesize, lat, arg);
        }

        ~GeneralCache() {
            for(int i = 0; i < _numsets; i++) {
                _sets[i].fini();
            }
            delete[] _sets;
        }

        virtual uint64_t getTag(uint64_t address) {
            return address & (~(_linesize - 1ULL));
        }

        virtual CacheLine *peek(uint64_t address) override {
            return _sets[INDEX::getSet(address, _linesize, _numsets)].peek(getTag(address));
        }

        virtual CacheLine *probe(uint64_t address, void *arg) override {
            CacheLine *line = _sets[INDEX::getSet(address, _linesize, _numsets)].probe(getTag(address), arg);
            return line;
        }

        virtual CacheLine *replace(uint64_t address, void *arg) override {
            CacheLine *repl = _sets[INDEX::getSet(address, _linesize, _numsets)].replace(getTag(address), arg);
            return repl;
        }

        virtual int latency() {
            return _latency;
        }

};

template<typename REPL> using Cache = GeneralCache<REPL, DefaultIndex>;

typedef Cache<NRU> NRUCache;
typedef Cache<RandomRepl> RandomCache;

}

#endif
