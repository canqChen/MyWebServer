
#ifndef EPOLLER_H
#define EPOLLER_H

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

#include "common/NoCopyable.h"

class EventLoop;

class Epoller : NoCopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    explicit
    Epoller(EventLoop* loop);
    ~Epoller();

    void poll(ChannelList& activeChannels, int timeoutMs = -1);
    void updateChannel(Channel* channel);

private:
    void __updateChannel(int op, Channel* channel);
    EventLoop* loop_;       // 所属eventloop
    std::vector<struct epoll_event> events_; // 监听事件列表
    int epollfd_;
};

#endif //EPOLLER_H