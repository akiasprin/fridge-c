#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include "SocketIO.hpp"

SocketIO::SocketIO() {
    kq = new KQueue();
    kq->handler_delegate = this;
}

bool SocketIO::listen() {
#define PORT 5005
#define MAXCONCURENCE 5
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    
    SocketIO::_set_tcp_nodelay(listenfd);
    SocketIO::_set_tcp_reuseaddr(listenfd);
    
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0) {
        return 1;
    }
    if (::listen(listenfd, MAXCONCURENCE) < 0) {
        return 1;
    }
    if (kq->Register(listenfd, 1, EVFILT_READ) < 0) { //注册监听可读事件
        return 1;
    }
    return 0;
}

void SocketIO::handle(struct kevent events[], int ret) {
    for (int i = 0; i < ret; i++) {
        struct kevent event = events[i];
        int sock = (int)event.ident;
        int16_t filter = event.filter;
        uint32_t flags = event.flags;
        intptr_t data = event.data;
        
        if (flags & EV_ERROR)               close(sock);
        else if (sock == listenfd)          accept(data);
        else if (filter == EVFILT_READ)     recv(sock, data);
        else if (filter == EVFILT_WRITE)    send(sock);
    }
}

bool SocketIO::accept(intptr_t size) {
    int clientfd;
    socklen_t client_addrlen;
    struct sockaddr_in client_addr;
    for (int i = 0 ; i < size; i++) {
        if ((clientfd =
             ::accept(listenfd, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) {
            return 1;
        }
        if (kq->Register(clientfd, 2, EVFILT_READ, EVFILT_WRITE) < 0) {
            return 1;
        }
    }
    return 0;
}

bool SocketIO::send(int sock) {
    return 0;
}

bool SocketIO::recv(int sock, intptr_t size) {
    char *buffer = (char *)malloc(size);
    ssize_t recvlen;
    if ((recvlen = ::recv(sock, buffer, size, 0)) <= 0) {
        close(sock);
    }
    printf("[%d]:%ld\n", sock, size);
    
    return 0;
}

bool SocketIO::close(int sock) {
    ::shutdown(sock, SHUT_RDWR);
    kq->Unregister(sock, 2, EVFILT_READ, EVFILT_WRITE);
    return 0;
}
