#include "TcpClient.h"
#include "../common/Log.h"
#include "EventLoop.h"
#include "TcpConnection.h"


TcpClient::TcpClient(EventLoop* loop, const InetAddress& peer)
        : loop_(loop), connected_(false), peer_(peer), 
          retryTimer_(nullptr), connector_(new Connector(loop, peer)),
          connectionCallback_(ConnectionCallback()),
          messageCallback_(MessageCallback())
{
    connector_->setNewConnectionCallback(std::bind(
            &TcpClient::__handleNewConnection, this, _1, _2, _3));
}

TcpClient::~TcpClient()
{
    if (connection_ && !connection_->isDisconnected())
        connection_->forceClose();
    if (retryTimer_ != nullptr) {
        loop_->cancelTimer(retryTimer_);
    }
}

void TcpClient::start()
{
    loop_->assertInLoopThread();
    connector_->start();
    retryTimer_ = loop_->runEvery(3s, [this](){ __retry(); });
}

void TcpClient::__retry()
{
    loop_->assertInLoopThread();
    if (connected_) {
        return;
    }

    LOG_WARN("TcpClient::__retry() reconnect %s...", peer_.toIpPort().c_str());
    // retry need to build another new connector
    connector_ = std::make_unique<Connector>(loop_, peer_);
    connector_->setNewConnectionCallback(std::bind(
            &TcpClient::__handleNewConnection, this, _1, _2, _3));
    connector_->start();
}

void TcpClient::__handleNewConnection(int connfd, const InetAddress& local, const InetAddress& peer)
{
    loop_->assertInLoopThread();
    // cancel timer after successfully connect to peer
    loop_->cancelTimer(retryTimer_);
    retryTimer_ = nullptr; // avoid duplicate cancel
    connected_ = true;
    auto conn = std::make_shared<TcpConnection>
            (loop_, connfd, local, peer);
    connection_ = conn;
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallBack(std::bind(
            &TcpClient::__handleCloseConnection, this, _1));
    // enable and tie channel
    conn->connectEstablished();
    connectionCallback_(conn);  // log
}

void TcpClient::__handleCloseConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    assert(connection_ != nullptr);
    connection_.reset();
    connectionCallback_(conn);  // log
}