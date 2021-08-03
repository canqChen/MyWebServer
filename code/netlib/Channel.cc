//
// Created by frank on 17-8-31.
//

#include <cassert>

#include "../eventloop/EventLoop.h"
#include "Channel.h"


Channel::Channel(EventLoop* loop, int fd)
        : polling(false), loop_(loop),
          fd_(fd), tied_(false),
          events_(0), revents_(0),
          handlingEvents_(false)
{}

Channel::~Channel() { 
    assert(!handlingEvents_); 
}

void Channel::handleEvents() {
    loop_->assertInLoopThread();
    // channel is always a member of another object
    // e.g. Timer, Acceptor, TcpConnection
    // TcpConnection is managed by std::shared_ptr,
    // and may be destructed when handling events,
    // so we use weak_ptr->shared_ptr to
    // extend it's life-time.
    if (tied_) {
        auto guard = tie_.lock();
        if (guard != nullptr) {
            __handleEventsWithGuard();
        }
    }
    else {
        __handleEventsWithGuard();
    }
}

void Channel::__handleEventsWithGuard() {
    handlingEvents_ = true;
    // EPOLLHUP表示读写都关闭，若此时无可读，直接关闭连接
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) 
            closeCallback_();
    }
    // 向已经断开的socket写或者读，会发生EPOLLERR，即表明已经断开，则关闭。属于服务器侧出错
    if (revents_ & EPOLLERR) {
        if (errorCallback_) 
            errorCallback_();
    }
    //  EPOLLPRI表示带外数据到来，EPOLLRDHUP表示读关闭，但是内核缓冲区还有数据可以读
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) 
            readCallback_();
    }
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) 
            writeCallback_();
    }
    handlingEvents_ = false;
}

// tie a Timer, Acceptor or TcpConnection
void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::__update() {
    loop_->updateChannel(this);
}

void Channel::__remove() {
    assert(polling);
    loop_->removeChannel(this);
}
