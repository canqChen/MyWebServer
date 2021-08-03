#include "Acceptor.h"

#include <unistd.h>
#include <cassert>

#include "EventLoop.h"
#include "../common/Log.h"
#include "Socket.h"


Acceptor::Acceptor(EventLoop* loop, const InetAddress& local, bool isLinger = false)
        : listening_(false), loop_(loop), lisentFd_(Socket::createSocket()),
          acceptChannel_(loop, lisentFd_), local_(local) 
{

    // timewait端口重用
    Socket::setReuseAddr(lisentFd_);

    // SO_REUSEPORT支持多个进程或者线程绑定到同一端口进行监听
    Socket::setReusePort(lisentFd_);

    // 设置是否优雅关闭
    // 优雅关闭: 直到所剩数据发送完毕或超时

    if(isLinger) {
        Socket::setLinger(lisentFd_, 5);
    }
    
    ret = ::bind(lisentFd_, local.getSockaddr(), local.getSocklen());
    if (ret == -1) {
        LOG_FATAL("Acceptor::bind() fail!");
    }
}

void Acceptor::listen() 
{
    loop_->assertInLoopThread();
    int ret = ::listen(lisentFd_, SOMAXCONN);
    if (ret == -1) {
        LOG_FATAL("Acceptor::listen() fail!");
    }

    acceptChannel_.setReadCallback([this](){__handleAcceptable();});
    acceptChannel_.enableRead();
}


Acceptor::~Acceptor() {
    ::close(lisentFd_);
}

void Acceptor::__handleAcceptable() {
    loop_->assertInLoopThread();

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    void* any = &addr;
    // SOCK_NONBLOCK 直接将accept的fd设为非阻塞
    int sockfd = ::accept4(lisentFd_, static_cast<sockaddr*>(any),
                           &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (sockfd == -1) {
        int savedErrno = errno;
        LOG_ERROR("Acceptor::accept4() fail!");
        switch (savedErrno) {
            case ECONNABORTED:
            case EINTR:
            case EAGAIN:
            case EMFILE:
                return;
            default:
                LOG_FATAL("unexpected accept4() error");
        }
    }

    if (newConnectionCallback_) {
        InetAddress peer;
        peer.setAddress(addr);
        newConnectionCallback_(sockfd, local_, peer);
    }
    else {
        ::close(sockfd);
    }
}