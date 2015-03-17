#include <simpleCacheController.h>

using namespace Memory;

void SimpleCacheController::AccessAction::execute() {

    BaseCache *cache = ctrl->get_cache();

    if(cache->get_port(e->type)) {

        int latency = cache->latency();
        CacheLine *line = cache->probe(e->address, NULL);
        Action *a;

        /**
         * register this access
         */
        ctrl->accesses.value++;
        if(e->type != REQUEST_UPDATE)
            ctrl->demand_accesses.value++;

        if(line) {
            /**
             * found line, no need to do any more probing
             */
            a = new HitAction(ctrl, e, line);

        } else {
            /**
             * register a miss action which handles the miss
             */
            a = new MissAction(ctrl, e);

        }

        ctrl->register_reuse_distance(e->address);

        ctrl->schedule(a, latency);

    } else {

        /**
         * port was busy this cycle repeat this action in 1 cycle
         */
        Action *a = new AccessAction(ctrl, e);
        ctrl->schedule(a, 1);

    }

}

void SimpleCacheController::SendAction::execute() {

    /**
     * if sending did not succeed, retry in 1 cycle
     */
    bool success = ctrl->send(message);
    if(!success) {
        SendAction *retry = new SendAction(ctrl, e, message);
        ctrl->schedule(retry, 1);
    }

}

void SimpleCacheController::RequestReplyAction::execute() {

    /**
     * if request was not a REQUEST_UPDATE
     * send reply to sender
     * in case of REQUEST_UPDATE, the sender does not expect a reply
     * since that is a "fire and forget" request
     */
    if(e->type == REQUEST_UPDATE)
        return;
    Message *reply = ctrl->create_reply(e->messageID, e->source);
    Action *send = new SendAction(ctrl, e, reply);
    ctrl->schedule(send, 0);
}

void SimpleCacheController::HandleDependencyAction::execute() {
    Action *a = new RequestReplyAction(ctrl, de);
    ctrl->schedule(a, 0);
}

static void update_line_state(CacheLine *line, QueueEntry *e) {
    OP_TYPE type = e->type;
    if(type == REQUEST_WRITE || type == REQUEST_UPDATE)
        line->state = LINE_MODIFIED;
    else if(line->state == LINE_INVALID)
        line->state = LINE_VALID;
    line->reused++;
}

static void init_line_state(CacheLine *line, QueueEntry *e) {
    OP_TYPE type = e->type;
    if(type == REQUEST_WRITE || type == REQUEST_UPDATE)
        line->state = LINE_MODIFIED;
    else
        line->state = LINE_VALID;
    line->reused = 0;
}

void SimpleCacheController::HitAction::execute() {
    update_line_state(line, e);

    /**
     * register this hit
     */
    ctrl->hits.value++;
    if(e->type != REQUEST_UPDATE)
        ctrl->demand_hits.value++;

    /**
     * access energy accounting
     */
    if(e->type == REQUEST_READ)
        ctrl->count_read_energy();
    else
        ctrl->count_write_energy();

    /**
     * reply back to this request
     */
    Action *a = new RequestReplyAction(ctrl, e);
    ctrl->schedule(a, 0);

    /**
     * handle any dependencies
     */
    ctrl->handle_deps(e);

}

void SimpleCacheController::MissAction::execute() {

//    printf("controller %s QueueEntry %p in MissAction\n", ctrl->get_name().c_str(), e);

    /**
     * create a READ request if request is READ or WRITE type
     * else create a UPDATE request
     * send it to the lower level interconnect
     */
    OP_TYPE lower_req_type = (e->type != REQUEST_UPDATE)?
        REQUEST_READ : REQUEST_UPDATE;

    Message *message = ctrl->create_request(e->address,
            lower_req_type, ctrl->get_lower_level());

    uint64_t messageID = message->id;

    Action *send = new SendAction(ctrl, e, message);
    ctrl->schedule(send, 0);

    /**
     * register handler to be called when message reply is received
     * wait for message only if request is not REQUEST_UPDATE
     */
    if(e->type != REQUEST_UPDATE) {
        Action *onReply = new MissReplyAction(ctrl, e);
        ctrl->wait(messageID, onReply);
    } else {
        // TODO: this is a hack, this can result in some lines not
        // being brought in if some requests depend on an UPDATE
        // request
        ctrl->handle_deps(e);
    }

    /**
     * increment the number of misses in this controller
     */
    ctrl->misses.value++;
    if(e->type != REQUEST_UPDATE)
        ctrl->demand_misses.value++;
}

void SimpleCacheController::MissReplyAction::execute() {

    /**
     * schedule an InsertAction
     */
    Action *a = new InsertAction(ctrl, e);
    ctrl->schedule(a, 0);

    /**
     * also schedule a RequestReplyAction
     */
    Action *reply = new RequestReplyAction(ctrl, e);
    ctrl->schedule(reply, 0);

    /**
     * handle all dependencies
     */
    ctrl->handle_deps(e);
}

void SimpleCacheController::InsertAction::execute() {

    BaseCache *cache = ctrl->get_cache();

    /**
     * try to get port, request is always request_update since this updates a line
     */
    if(cache->get_port(REQUEST_UPDATE)) {
        CacheLine *line = cache->replace(e->address, NULL);

        if(line->state == LINE_MODIFIED) {
            /**
             * if line was modified, schedule a request to update it in the lower level
             */
            Action *evict = new EvictAction(ctrl, e, line->tag);
            ctrl->schedule(evict, cache->latency());
        }

        ctrl->register_reuse(line->reused);

        line->tag = cache->getTag(e->address);
        init_line_state(line, e);

        /**
         * increment the number of inserts in the cache controller
         */
        ctrl->inserts.value++;

        /**
         * insertion energy accounting
         */
        ctrl->count_write_energy();

    } else {

        Action *retry = new InsertAction(ctrl, e);
        ctrl->schedule(retry, 1);

    }
}

void SimpleCacheController::EvictAction::execute() {

    Message *message = ctrl->create_request(address, REQUEST_UPDATE,
            ctrl->get_lower_level());
    Action *send = new SendAction(ctrl, e, message);
    ctrl->schedule(send, 0);

    ctrl->writebacks.value++;

    /**
     * have to read before doing writeback
     */
    ctrl->count_read_energy();
}
