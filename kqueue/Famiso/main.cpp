#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include "SocketIO.hpp"

using namespace std;

int main() {
    SocketIO *sokit = new SocketIO();
    sokit->listen();
    sokit->kq->Loop();
    return 0;
}
