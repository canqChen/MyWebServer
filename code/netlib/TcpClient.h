#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "Callbacks.h"
#include "Connector.h"
#include "Timer.h"

class TcpClient: NoCopyable
{
public:
    TcpClient(EventLoop* loop, const InetAddress& peer);
    ~TcpClient();

    void start();
    void setConnectionCallback(const ConnectionCallback& cb)
    { 
        connectionCallback_ = cb; 
    }
    void setMessageCallback(const MessageCallback& cb)
    { 
        messageCallback_ = cb; 
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { 
        writeCompleteCallback_ = cb;
    }
    void setErrorCallback(const EventCallback& cb)
    {
        connector_->setErrorCallback(cb); 
    }

private:
    void __retry();
    void __handleNewConnection(int connfd, const InetAddress& local, const InetAddress& peer);
    void __handleCloseConnection(const TcpConnectionPtr& conn);

private:
    typedef std::unique_ptr<Connector> ConnectorPtr;

    EventLoop* loop_;
    bool connected_;
    const InetAddress peer_;
    Timer* retryTimer_;
    ConnectorPtr connector_;
    TcpConnectionPtr connection_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};


#endif //TCPCLIENT_H
