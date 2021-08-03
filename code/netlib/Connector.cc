#include <unistd.h>
#include <sys/socket.h>
#include <cassert>

#include "./EventLoop.h"
#include "../common/Log.h"
#include "./Connector.h"
#include "./Socket.h"


Connector::Connector(EventLoop* loop, const InetAddress& peer)
        : loop_(loop), peer_(peer), sockfd_(Socket::createSocket()),
          connected_(false), started_(false), channel_(loop, sockfd_)
{
    channel_.setWriteCallback([this](){ __handleWrite();});
}

Connector::~Connector()
{
    if (!connected_)
        ::close(sockfd_);
}

void Connector::start()
{
    loop_->assertInLoopThread();
    assert(!started_);
    started_ = true;

    // 若建立连接成功，epoll的结果中该描述符可写；若失败，则可写可读，此时可以使用getsockopt获取错误信息
    int ret = ::connect(sockfd_, peer_.getSockaddr(), peer_.getSocklen());
    if (ret == -1) {
        if (errno != EINPROGRESS)
            __handleWrite();
        else
            channel_.enableWrite();  // EINPROGRESS, 连接中，通过epoll判断socket是否可写来确定连接是否成功
    }
    else __handleWrite();
}

void Connector::__handleWrite()
{
    loop_->assertInLoopThread();
    assert(started_);

    // 将socket移出epoll监听列表
    loop_->removeChannel(&channel_);
    // 出现可写事件，不一定是连接成功，需要通过检查so_error判断
    // 如果可写，且so_error为0，则表示连接成功；
    // 如果是可读可写，此时so_error不为0，则连接失败
    int err;
    int ret = Socket::getSocketError(sockfd_, &err);
    if (ret == 0)
        errno = err;
    if (errno != 0) {   // so_error不为0，连接失败，执行errorCallback_
        LOG_ERROR("Connector::connect()");
        if (errorCallback_)
            errorCallback_();
    }
    else if (newConnectionCallback_) {  // so_error为0，执行连接成功后的newConnectionCallback_
        struct sockaddr_in addr;
        len = sizeof(addr);
        void* any = &addr;
        ret = ::getsockname(sockfd_, static_cast<sockaddr*>(any), &len);
        if (ret == -1)
            LOG_ERROR("Connection::getsockname()");
        InetAddress local;
        local.setAddress(addr);

        // now sockfd_ is not belong to us
        connected_ = true;
        newConnectionCallback_(sockfd_, local, peer_);
    }
}
