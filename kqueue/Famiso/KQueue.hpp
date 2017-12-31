#pragma once
class SocketIO;

class KQueue {
public:
    int kq;
    SocketIO *handler_delegate;
    KQueue();
    
    int Register(int fd, int n, ...);
    int Unregister(int fd, int n, ...);
    void Loop();
};
