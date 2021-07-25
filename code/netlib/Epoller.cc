

#include <unistd.h>
#include <cassert>

#include "../common/Log.h"
#include "./EventLoop.h"
#include "./Epoller.h"
#include "./Channel.h"

Epoller::Epoller(EventLoop* loop)
        :loop_(loop),
         events_(1024),
         epollfd_(::epoll_create1(EPOLL_CLOEXEC)) {
    if (epollfd_ == -1) {
        LOG_FATAL("Create epoll fail in %s", "Epoller::epoll_create1");
    }
}

Epoller::~Epoller() {
    ::close(epollfd_);
}

void Epoller::poll(ChannelList& activeChannels, int timeoutMs = -1) {
    loop_->assertInLoopThread();
    int maxEvents = static_cast<int>(events_.size());
    int nEvents = epoll_wait(epollfd_, events_.data(), maxEvents, timeoutMs);
    if (nEvents == -1) {
        if (errno != EINTR) {
            LOG_ERROR("Fail in %s", "Epoller::poll");
            exit(1);
        }
    }
    else if (nEvents > 0) {
        for (int i = 0; i < nEvents; ++i) {
            auto channel = static_cast<Channel*>(events_[i].data.ptr);
            channel->setRealEvents(events_[i].events);
            activeChannels.push_back(channel);
        }
        if (nEvents == maxEvents)
            events_.resize(2 * events_.size());
    }
}

void Epoller::updateChannel(Channel* channel) {
    loop_->assertInLoopThread();
    int op = 0;
    if (!channel->isPooling()) { // 如果不被监听状态，加入监听列表
        assert(!channel->isNoneEvents());
        op = EPOLL_CTL_ADD;
        channel->setPooling(true);
    }
    else if (!channel->isNoneEvents()) {   // 监听事件非空，mod
        op = EPOLL_CTL_MOD;
    }
    else {  // 监听事件空，删除
        op = EPOLL_CTL_DEL;
        channel->setPooling(false);
    }
    __updateChannel(op, channel);
}

void Epoller::__updateChannel(int op, Channel* channel) {
    struct epoll_event ee;
    ee.events = channel->getEvents();
    ee.data.ptr = channel;
    int ret = ::epoll_ctl(epollfd_, op, channel->getFd(), &ee);
    if (ret == -1) {
        LOG_FATAL("epoll_ctl fail in Epoller::__updateChannel, op: %d", op);
    }
}