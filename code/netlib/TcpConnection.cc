#include "TcpConnection.h"
#include <cassert>
#include <unistd.h>

#include "../common/Log.h"
#include "EventLoop.h"
#include "Socket.h"


TcpConnection::TcpConnection(EventLoop *loop, int sockfd,
                             const InetAddress& local,
                             const InetAddress& peer)
        : loop_(loop), sockfd_(sockfd),
          channel_(loop, sockfd_), state_(CONNECTING),
          local_(local), peer_(peer), highWaterMark_(0)
{
    // set callbacks in channel
    channel_.setReadCallback([this](){__handleRead();});
    channel_.setWriteCallback([this](){__handleWrite();});
    channel_.setCloseCallback([this](){__handleClose();});
    channel_.setErrorCallback([this](){__handleError();});

    LOG_TRACE("New TcpConnection() %s fd=%d", name().c_str(), sockfd);
}

TcpConnection::~TcpConnection() 
{
    assert(state_ == DISCONNECTED);
    ::close(sockfd_);

    LOG_DEBUG("~TcpConnection() %s close fd=%d", name().c_str(), sockfd_);
}

// tie to channal and enable read
void TcpConnection::connectEstablished()
{
    assert(state_ == CONNECTING);
    state_ = CONNECTED;
    // 延长tcpconnection的生命周期，防止在channel处理完事件前tcpconnection被释放
    channel_.tie(shared_from_this());
    channel_.enableRead();
}

bool TcpConnection::isConnected() const 
{ 
    return state_ == CONNECTED; 
}

bool TcpConnection::isDisconnected() const 
{ 
    return state_ == DISCONNECTED; 
}

void TcpConnection::send(std::string_view data) 
{
    send(data.data(), data.length());
}

void TcpConnection::send(const char *data, size_t len)
{
    if (state_ != CONNECTED) {
        LOG_WARN("TcpConnection::send() not connected, give up send");
        return;
    }
    if (loop_->isInLoopThread()) {
        __sendInLoop(data, len);
    }
    else {
        loop_->queueInLoop(
                [ptr = shared_from_this(), str = std::string(data, data + len)]()
                { ptr->__sendInLoop(str);});
    }
}

void TcpConnection::__sendInLoop(const char *data, size_t len) 
{
    loop_->assertInLoopThread();
    // DISCONNECTING is OK
    if (state_ == DISCONNECTED) {
        LOG_WARN("TcpConnection::__sendInLoop() disconnected, give up send");
        return;
    }
    ssize_t n = 0;
    size_t remain = len;
    bool faultError = false;
    // isWriting means there are other message to send, do not write to avoid chaos
    if (!channel_.isWriting()) {
        assert(outputBuffer_.readableBytes() == 0);
        n = ::write(sockfd_, data, len);
        if (n == -1) {
            if (errno != EAGAIN) {
                LOG_ERROR("TcpConnection::write() error");
                if (errno == EPIPE || errno == ECONNRESET)
                    faultError = true;
            }
            n = 0;
        }
        else {
            remain -= static_cast<size_t>(n);
            if (remain == 0 && writeCompleteCallback_) {
                // user may send data in writeCompleteCallback_
                // queueInLoop can break the chain
                loop_->queueInLoop(std::bind(
                        writeCompleteCallback_, shared_from_this()));
            }
        }
    }
    // TODO: add highWaterMarkn
    if (!faultError && remain > 0) {
        if (highWaterMarkCallback_) {
            size_t oldLen = outputBuffer_.readableBytes();
            size_t newLen = oldLen + remain;
            if (oldLen < highWaterMark_ && newLen >= highWaterMark_)
                loop_->queueInLoop(std::bind(
                        highWaterMarkCallback_, shared_from_this(), newLen));
        }
        // can not be sent at one time, append the remain message 
        // to the outputbuff, and write in callback
        outputBuffer_.append(data + n, remain);
        channel_.enableWrite();
    }
}

void TcpConnection::__sendInLoop(const std::string& message) 
{
    __sendInLoop(message.data(), message.size());
}

void TcpConnection::send(Buffer& buffer) 
{
    if (state_ != CONNECTED) {
        LOG_WARN("TcpConnection::send() not connected, give up send");
        return;
    }
    if (loop_->isInLoopThread()) {
        __sendInLoop(buffer.readPtr(), buffer.readableBytes());
        buffer.forwardReadPos(buffer.writePtr());
    }
    else {
        loop_->queueInLoop(
                [ptr = shared_from_this(), str = buffer.retrieveAll()]()
                { ptr->__sendInLoop(str); });
    }
}

void TcpConnection::shutdownWR() 
{
    assert(state_ <= DISCONNECTING);
    if (__stateAtomicGetAndSet(DISCONNECTING) == CONNECTED) {
        if (loop_->isInLoopThread())
            __shutdownWRInLoop();
        else {
            loop_->queueInLoop(std::bind(
                    &TcpConnection::__shutdownWRInLoop, shared_from_this()));
        }
    }
}

void TcpConnection::__shutdownWRInLoop() 
{
    loop_->assertInLoopThread();
    if (state_ != DISCONNECTED && !channel_.isWriting()) {
        if (::shutdown(sockfd_, SHUT_WR) == -1)
            LOG_ERROR("TcpConnection:shutdown() error");
    }
}

void TcpConnection::forceClose() 
{
    if (state_ != DISCONNECTED) {
        if (__stateAtomicGetAndSet(DISCONNECTING) != DISCONNECTED) {
            loop_->queueInLoop(std::bind(
                    &TcpConnection::__forceCloseInLoop, shared_from_this()));
        }
    }
}

void TcpConnection::__forceCloseInLoop() 
{
    loop_->assertInLoopThread();
    if (state_ != DISCONNECTED) {
        __handleClose();
    }
}

void TcpConnection::disableRead() 
{
    loop_->runInLoop([this]() {
        if (channel_.isReading())
            channel_.disableRead();
    });
}

void TcpConnection::enableRead() 
{
    loop_->runInLoop([this]() {
        if (!channel_.isReading())
            channel_.enableRead();
    });
}

int TcpConnection::__stateAtomicGetAndSet(int newState) 
{
    return __atomic_exchange_n(&state_, newState, __ATOMIC_SEQ_CST);
}

void TcpConnection::__handleRead() 
{
    loop_->assertInLoopThread();
    assert(state_ != DISCONNECTED);
    int savedErrno;
    ssize_t n = inputBuffer_.readFd(sockfd_, &savedErrno);
    if (n == -1) {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::read() error");
        __handleError();
    }
    else if (n == 0) {
        __handleClose();
    }
    else {  // process upper-level logic
        auto guardThis = shared_from_this();
        messageCallback_(guardThis, inputBuffer_);
    }
}

void TcpConnection::__handleWrite() 
{
    if (state_ == DISCONNECTED) {
        LOG_WARN("TcpConnection::__handleWrite() disconnected, "
                     "give up writing %lu bytes", outputBuffer_.readableBytes());
        return;
    }
    assert(outputBuffer_.readableBytes() > 0);
    assert(channel_.isWriting());
    ssize_t n = ::write(sockfd_, outputBuffer_.readPtr(), outputBuffer_.readableBytes());
    if (n == -1) {
        LOG_ERROR("TcpConnection::write() error");
    }
    else {
        outputBuffer_.forwardReadPos(static_cast<size_t>(n));
        if (outputBuffer_.readableBytes() == 0) {
            // write complete, disable to listen writable event, 
            // otherwise writable event will be aroused even if 
            // there are no data to send
            channel_.disableWrite();
            // write complete, and going to shutdown, send FIN
            if (state_ == DISCONNECTING)
                __shutdownWRInLoop();
            if (writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(
                        writeCompleteCallback_, shared_from_this()));
            }
        }
    }
}

void TcpConnection::__handleClose() 
{
    loop_->assertInLoopThread();
    assert(state_ == CONNECTED ||
           state_ == DISCONNECTING);
    state_ = DISCONNECTED;
    loop_->removeChannel(&channel_);
    // delete this tcpconnetion from tcp server or tcp client
    closeCallback_(shared_from_this());
}

void TcpConnection::__handleError()
{
    int err;
    socklen_t len = sizeof(err);
    int ret = Socket::getSocketError(sockfd_, &err);
    // int ret = getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &err, &len);
    if (ret != -1)
        errno = err;
    LOG_ERROR("TcpConnection::__handleError()");
}
