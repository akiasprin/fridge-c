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
#include "KQueue.hpp"
#include "SocketIO.hpp"

KQueue::KQueue() {
    this->kq = kqueue();
}

int KQueue::Register(int fd, int n, ...) {
    va_list vap;
    va_start(vap , n);
    struct kevent *changes = new struct kevent[n];
    for (int i = 0; i < n; i++) {
        EV_SET(changes + i, fd, va_arg(vap, int), EV_ADD, 0, 0, NULL);
    }
    va_end(vap);
    return kevent(this->kq, changes, 1, NULL, 0, NULL);
}

int KQueue::Unregister(int fd, int n, ...) {
    va_list vap;
    va_start(vap , n);
    struct kevent *changes = new struct kevent[n];
    for (int i = 0; i < n; i++) {
        EV_SET(changes + i, fd, va_arg(vap, int), EV_DELETE, 0, 0, NULL);
    }
    va_end(vap);
    return kevent(this->kq, changes, 1, NULL, 0, NULL);
}

void KQueue::Loop() {
#define MAX_EVENT_COUNT 1024
    struct timespec timeout = { 5, 0 }; //
    struct kevent events[MAX_EVENT_COUNT];
    for (;;) {
        int ret = kevent(kq, NULL, 0, events, MAX_EVENT_COUNT, &timeout);
        printf("[kqueue pop]:%d\n", ret);
        handler_delegate->handle(events, ret);
    }
}


