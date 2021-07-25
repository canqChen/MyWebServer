#include <unistd.h>
#include <cassert>

#include "./EventLoop.h"
#include "../common/Log.h"
#include "./Acceptor.h"


int Acceptor::createSocket() {
    int ret = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (ret == -1) {
        LOG_FATAL("Create socket fail in %s", "Acceptor::createSocket() !");
    }
    return ret;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& local, bool isLinger = false)
        : listening_(false), loop_(loop), lisentFd_(createSocket()),
          acceptChannel_(loop, lisentFd_), local_(local) {
    int on = 1;
    // timewait端口重用
    int ret = ::setsockopt(lisentFd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        LOG_FATAL("Acceptor::setsockopt() SO_REUSEADDR fail!");
    }
    // SO_REUSEPORT支持多个进程或者线程绑定到同一端口进行监听
    ret = ::setsockopt(lisentFd_, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    if (ret == -1) {
        LOG_FATAL("Acceptor::setsockopt() SO_REUSEPORT fail!");
    }
    // 设置是否优雅关闭
    // 优雅关闭: 直到所剩数据发送完毕或超时
    struct linger optLinger = { 0 };
    if(isLinger) {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 5; // 单位：s
    }
    
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_FATAL("Set linger error!");
        return false;
    }
    ret = ::bind(lisentFd_, local.getSockaddr(), local.getSocklen());
    if (ret == -1) {
        LOG_FATAL("Acceptor::bind() fail!");
    }
}

void Acceptor::listen() {
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