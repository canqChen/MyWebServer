#include "Epoller.h"

Epoller::Epoller(int maxEvent):_epollFd(epoll_create(512)), _events(maxEvent){
    assert(_epollFd >= 0 && _events.size() > 0);
}

Epoller::~Epoller() {
    close(_epollFd);
}

// 向epoll事件表中注册监听事件
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev);
}

// 修改fd指向文件描述符的监听事件
bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev);
}

// 从epoll事件表中删除fd注册的监听事件
bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &ev);
}

// 执行epoll_wait，设定监听超时值，单位ms，-1为持续等待直至事件发生，返回就绪事件数
int Epoller::Wait(int timeoutMs) {
    return epoll_wait(_epollFd, &_events[0],   // 传递events底层封装的内存首地址，内核将就绪队列中的事件复制到events_中
            static_cast<int>(_events.size()), timeoutMs);
}

// 根据就绪事件列表event_下标获得事件对应fd
int Epoller::GetFdByEvent(size_t i) const {
    assert(i < _events.size() && i >= 0);
    return _events[i].data.fd;
}

// 根据就绪事件列表event_下标获得对应事件类型
uint32_t Epoller::GetEvent(size_t i) const {
    assert(i < _events.size() && i >= 0);
    return _events[i].events;
}