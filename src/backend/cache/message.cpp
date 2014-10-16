#include <message.h>
#include <controller.h>
#include <cstdio>

using namespace Memory;
using namespace std;

void Message::print(FILE *fp) {
        fprintf(fp, "id %6" PRId64 "   source %s   dest %s   replyTo %6" PRId64 "\n",
                id, source->get_name().c_str(), dest->get_name().c_str(), replyTo);
}

void Message::print() {
    print(stdout);
}

