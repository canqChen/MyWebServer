#include "TcpServer.h"
#include "../common/Log.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "../common/Buffer.h"


TcpServer::TcpServer(EventLoop* loop, const InetAddress& local)
        : loop_(loop), acceptor_(loop, local)
{
    // call back while a new connection was built in the acceptor
    acceptor_.setNewConnectionCallback(std::bind(
            &TcpServer::__handleNewConnection, this, _1, _2, _3));
}

// let acceptor start to listen the local address
void TcpServer::startListen()
{
    acceptor_.listen();
}

void TcpServer::__handleNewConnection(int connfd,
                                    const InetAddress& local,
                                    const InetAddress& peer)
{
    loop_->assertInLoopThread();
    auto conn = std::make_shared<TcpConnection>
            (loop_, connfd, local, peer);
    connections_.insert(conn);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallBack(std::bind(
            &TcpServer::__handleCloseConnection, this, _1));
    // enable and tie channel
    conn->connectEstablished();

    // upper callback: log info
    connectionCallback_(conn);
}

void TcpServer::__handleCloseConnection(const TcpConnectionPtr& conn) 
{
    loop_->assertInLoopThread();
    size_t ret = connections_.erase(conn);
    assert(ret == 1);
    // log info
    connectionCallback_(conn);
}

