#ifndef TINYEV_CHANNEL_H
#define TINYEV_CHANNEL_H

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include "../common/NoCopyable.h"
#include "Callbacks.h"
#include "EventLoop.h"


// 每个channel对象只属于一个evenloop，且只负责一个fd的io事件分发给不同的回调，管理fd，不拥有fd，fd生命周期与channel无关
class Channel: NoCopyable {
public:
    typedef std::function<void()> EventCallback;
    Channel(EventLoop* loop, int fd);
    ~Channel();
    // 注册可读回调
    void setReadCallback(const EventCallback& cb) 
    { 
        readCallback_ = cb;
    }

    // 注册可写回调
    void setWriteCallback(const EventCallback& cb) 
    { 
        writeCallback_ = cb; 
    }

    // 注册关闭fd回调
    void setCloseCallback(const EventCallback& cb) 
    { 
        closeCallback_ = cb; 
    }
    // 注册出错时回调
    void setErrorCallback(const EventCallback& cb) 
    { 
        errorCallback_ = cb; 
    }
    // 根据就绪事件分发给对应回调函数处理
    void handleEvents();

    int getFd() const { 
        return fd_; 
    }

    // fd事件为空
    bool isNoneEvents() const 
    { 
        return events_ == 0; 
    }
    // fd事件列表
    unsigned getEvents() const 
    { 
        return events_; 
    }

    void setRealEvents(unsigned revents) 
    { 
        revents_ = revents; 
    }

    void tie(const std::shared_ptr<void>& obj);

    // 监听可读事件
    void enableRead() { 
        events_ |= (EPOLLIN | EPOLLPRI); 
        __update();
    }
    // 监听可写事件
    void enableWrite() { 
        events_ |= EPOLLOUT; 
        __update();
    }
    // 取消监听可读事件
    void disableRead() { 
        events_ &= ~EPOLLIN; 
        __update(); 
    }

    // 取消监听可写事件
    void disableWrite() { 
        events_ &= ~EPOLLOUT; 
        __update();
    }

    void disableAll() { 
        events_ = 0; 
        __update();
    }
    // 是否正在监听可读事件
    bool isReading() const { 
        return events_ & EPOLLIN; 
    }
    // 是否正在监听可写事件
    bool isWriting() const { 
        return events_ & EPOLLOUT;
    }
    // 是否正在被监听
    bool isPooling() const {
        return polling_;
    }

    void setPooling(bool pooling) {
        polling_ = pooling;
    }
private:
    // 通过所属eventloop调用epoller更新fd监听事件
    void __update();
    // 
    void __remove();
    // 处理io事件底层封装函数
    void __handleEventsWithGuard();

private:
    EventLoop* loop_;   // 所属eventloop
    int fd_;    // 所负责的fd

    bool polling_;

    std::weak_ptr<void> tie_;
    bool tied_;
    // 监听事件
    unsigned events_;
    // 实际发生的事件
    unsigned revents_;

    // 是否正在处理io事件中
    bool handlingEvents_;


    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};


#endif //TINYEV_CHANNEL_H
