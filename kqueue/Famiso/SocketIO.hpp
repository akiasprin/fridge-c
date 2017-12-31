#pragma once
#include <netinet/tcp.h>
#include "KQueue.hpp"

class SocketIO {
public:
    int listenfd;
    KQueue *kq;
    SocketIO();
    
    bool listen();
    bool accept(intptr_t size);
    bool send(int sock);
    bool recv(int sock, intptr_t size);
    bool close(int sock);
    void handle(struct kevent events[], int ret);
    
    static void _set_tcp_nodelay(int fd) {
        int enable = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
    }
    
    static void _set_tcp_reuseaddr(int fd) {
        int enable = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&enable, sizeof(int));
    }

};

