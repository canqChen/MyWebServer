#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <any>

#include "../common/NoCopyable.h"
#include "../common/Buffer.h"
#include "Callbacks.h"
#include "Channel.h"
#include "InetAddress.h"

// ont object for each tcp connection
class TcpConnection: NoCopyable, public std::enable_shared_from_this<TcpConnection> 
{
public:
    TcpConnection(EventLoop* loop, int sockfd,
                  const InetAddress& local,
                  const InetAddress& peer);
    ~TcpConnection();

    void setMessageCallback(const MessageCallback& cb) 
    { 
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) 
    { 
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t mark) 
    { 
        highWaterMarkCallback_ = cb; 
        highWaterMark_ = mark; 
    }

    // internal use
    void setCloseCallBack(const CloseCallback& cb) 
    { 
        closeCallback_ = cb; 
    }

    // TcpServer
    void connectEstablished();

    bool isConnected() const;
    bool isDisconnected() const;

    const InetAddress& local() const 
    { 
        return local_; 
    }

    const InetAddress& peer() const 
    { 
        return peer_; 
    }

    std::string name() const 
    { 
        return peer_.toIpPort() + " -> " + local_.toIpPort(); 
    }

    void setContext(const std::any& context) 
    { 
        context_ = context; 
    }

    const std::any& getContext() const 
    { 
        return context_; 
    }

    std::any& getContext() 
    { 
        return context_; 
    }

    // I/O operations are thread safe
    void send(std::string_view data);
    void send(const char* data, size_t len);
    void send(Buffer& buffer);
    void shutdownWR();
    void forceClose();

    void disableRead();
    void enableRead();

    // not thread safe
    bool isReading() 
    { 
        return channel_.isReading(); 
    };

    const Buffer& inputBuffer() const 
    { 
        return inputBuffer_; 
    }
    const Buffer& outputBuffer() const 
    { 
        return outputBuffer_; 
    }

private:
    enum ConnectionState 
    {
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        DISCONNECTED
    };

private:
    void __handleRead();
    void __handleWrite();
    void __handleClose();
    void __handleError();

    void __sendInLoop(const char* data, size_t len);
    void __sendInLoop(const std::string& message);
    void __shutdownWRInLoop();
    void __forceCloseInLoop();

    // cas spin lock
    int __stateAtomicGetAndSet(int newState);

private:
    // TODO: use struct iovec
    EventLoop* loop_;
    const int sockfd_;
    Channel channel_;
    int state_;
    InetAddress local_;  // tcp local address
    InetAddress peer_;  // tcp client address
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    size_t highWaterMark_;
    std::any context_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;   // serves for tcpserver and tcpclient
};


#endif //TCPCONNECTION_H
