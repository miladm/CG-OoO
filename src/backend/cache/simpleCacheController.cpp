#include <simpleCacheController.h>

using namespace std;
using namespace Memory;


void SimpleCacheController::request_cb(Message *message) {

    /**
     * for a request, create a QueueEntry and do an AccessAction
     */
    QueueEntry *e = new QueueEntry(message);

    uint64_t tag = cacheLines_->getTag(message->address);

    if(per_entry_deps.count(tag)) {

//        printf("controller %s message %p has dependent entry\n",
//                get_name().c_str(), message);
        per_entry_deps[tag].push_back(e);

        QueueEntry *oe = original_entry[tag];

        /**
         * if the original request is READ/WRITE and the new request is
         * WRITE/UPDATE, we change the original request to be WRITE,
         * since WRITE will invoke a reply and UPDATE will not
         *
         * however, in case the original request is UPDATE, we do not change
         * its type, since otherwise an extraneous reply will be generated
         */
        if(e->type == REQUEST_WRITE || e->type == REQUEST_UPDATE) {
            if(oe->type != REQUEST_UPDATE) {
                oe->type = REQUEST_WRITE;
            }
        }

    } else {

        Action *a = new AccessAction(this, e);
        Controller::schedule(a, 0);
        per_entry_deps[tag] = DependencyList();
        original_entry[tag] = e;

//        TODO - USEFUL FOR DEBUGGING CACHE
//        printf("controller %s message %p going to access\n",
//                get_name().c_str(), message);

    }

}

void SimpleCacheController::handle_deps(QueueEntry *e) {
    uint64_t tag = cacheLines_->getTag(e->address);

    for(auto de : per_entry_deps[tag]) {
        Action *a = new HandleDependencyAction(this, e, de);
        Controller::schedule(a, 0);
    }

    per_entry_deps.erase(tag);
    original_entry.erase(tag);

}

void SimpleCacheController::register_reuse(int reused) {
    // reuse frequency bins: 0, 1, 2, > 2
    if(reused > 2)
        reused = 3;
    reuse_frequencies.value[reused]++;
}

void SimpleCacheController::register_reuse_distance(uint64_t address) {
    timestamp++;
    uint64_t line = address >> 6;
    int distance = timestamp - last_accessed[line];
    last_accessed[line] = timestamp;
    int bin = vlog2(distance);
    if(bin >= 32)
        bin = 31;
    reuse_distances.value[bin]++;
}

void SimpleCacheController::init(const YAML::Node &root) {
    const YAML::Node &cachenode = root["cache"];
    int size, assoc, linesize, latency;
    cachenode["size"] >> size;
    cachenode["assoc"] >> assoc;
    cachenode["linesize"] >> linesize;
    cachenode["latency"] >> latency;
    cacheLines_ = new NRUCache(
            size,
            assoc,
            linesize,
            latency,
            NULL);

    accesses.init("accesses", this);
    hits.init("hits", this);
    misses.init("misses", this);
    demand_accesses.init("demand_accesses", this);
    demand_hits.init("demand_hits", this);
    demand_misses.init("demand_misses", this);
    inserts.init("inserts", this);
    writebacks.init("writebacks", this);

    reuse_frequencies.init("reuse_frequencies", this, 4);
    reuse_distances.init("reuse_distances", this, 32);

    timestamp = 0;
}

void SimpleCacheController::dump_stats(FILE *fp) {
    accesses.print(fp);
    hits.print(fp);
    misses.print(fp);
    demand_accesses.print(fp);
    demand_hits.print(fp);
    demand_misses.print(fp);
    inserts.print(fp);
    writebacks.print(fp);
    reuse_frequencies.print(fp);
    reuse_distances.print(fp);
}

Generator<Controller, SimpleCacheController> simple_gen("simple");
