#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <unordered_set>

#include "./Callbacks.h"
#include "./Acceptor.h"


class EventLoop;
// one evenloop per server, run the loop in a thread
class TcpServer : NoCopyable
{
public:
    TcpServer(EventLoop *loop, const InetAddress &local);

    void setConnectionCallback(const ConnectionCallback &cb) { 
        connectionCallback_ = cb; 
    }

    void setMessageCallback(const MessageCallback &cb) { 
        messageCallback_ = cb; 
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { 
        writeCompleteCallback_ = cb; 
    }

    void startListen();

private:
    void __handleNewConnection(int connfd, const InetAddress &local, const InetAddress &peer);

    void __handleCloseConnection(const TcpConnectionPtr &conn);

    typedef std::unordered_set<TcpConnectionPtr> ConnectionSet;

    EventLoop *loop_;
    Acceptor acceptor_;
    ConnectionSet connections_;     // maintain tcp connections charged by this server/evenloop
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};


#endif //TCPSERVER_H
