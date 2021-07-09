#include "Epoller.h"

Epoller::Epoller(int maxEvent):epollFd_(epoll_create(512)), events_(maxEvent){
    assert(epollFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epollFd_);
}

// 向epoll事件表中注册监听事件
bool Epoller::addFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

// 修改fd指向文件描述符的监听事件
bool Epoller::modFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}

// 从epoll事件表中删除fd注册的监听事件
bool Epoller::delFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}

// 执行epoll_wait，设定监听超时值，单位ms，-1为持续等待直至事件发生，返回就绪事件数
int Epoller::wait(int timeoutMs) {
    return epoll_wait(epollFd_, &events_[0],   // 传递events底层封装的内存首地址，内核将就绪队列中的事件复制到events_中
            static_cast<int>(events_.size()), timeoutMs);
}

// 根据就绪事件列表event_下标获得事件对应fd
int Epoller::getFdByEvent(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

// 根据就绪事件列表event_下标获得对应事件类型
uint32_t Epoller::getEvent(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}