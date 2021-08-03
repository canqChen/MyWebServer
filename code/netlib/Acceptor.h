#ifndef ACCEPTOR_H
#define ACCEPTOR_H


#include "../common/NoCopyable.h"
#include "Callbacks.h"
#include "Channel.h"
#include "InetAddress.h"

class EventLoop;

class Acceptor:NoCopyable {
public:
    Acceptor(EventLoop* loop, const InetAddress& local, bool isLinger = false);
    ~Acceptor();

    bool isListening() const { 
        return listening_; 
    }

    void listen();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { 
        newConnectionCallback_ = cb; 
    }

private:
    void __handleAcceptable();

    bool listening_;
    EventLoop* loop_;
    const int lisentFd_;
    Channel acceptChannel_;
    InetAddress local_;
    NewConnectionCallback newConnectionCallback_;
};


#endif