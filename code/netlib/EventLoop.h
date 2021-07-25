#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "../common/NoCopyable.h"
#include "./Timer.h"
#include "./Epoller.h"

class Channel;

// one loop per thread
class EventLoop : NoCopyable
{
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void quit(); // thread safe

    // 将其他线程调用的应该在本线程执行的任务入队，由__doPendingTasks处理
    void runInLoop(const Task& task);
    void runInLoop(Task&& task);

    // 调用把实际工作转移到io线程执行
    void queueInLoop(const Task& task);
    void queueInLoop(Task&& task);

    // 计划任务
    Timer* runAt(Timestamp when, TimerCallback callback);
    Timer* runAfter(Nanosecond interval, TimerCallback callback);
    Timer* runEvery(Nanosecond interval, TimerCallback callback);

    // 取消计划任务
    void cancelTimer(Timer* timer);

    // 入队后需要唤醒，唤醒方式是向eventfd写入，然后epoll_wait返回，执行__doPendingTasks
    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    // 调用该断言的成员函数只能在特定io线程中执行
    void assertInLoopThread();
    void assertNotInLoopThread();
    bool isInLoopThread();

private:
    // 处理其他线程调用的应该在本线程执行的任务
    void __doPendingTasks();
    void __handleWakeUpFdRead();
    const pid_t tid_;  // 本loop所属线程id
    std::atomic_bool quit_; 
    bool doingPendingTasks_;    // 判断是否正在执行pendingTasks_中的任务
    Epoller poller_;    // one epoller per thread
    Epoller::ChannelList activeChannels_;
    const int wakeupFd_;    // 用于唤醒epoll_wait
    Channel wakeupChannel_; //wakeupFd_所托管channel
    std::mutex mutex_;  // 保护pendingTasks_入队出队
    std::vector<Task> pendingTasks_; // guarded by mutex_
    TimerQueue timerQueue_; // one timerqueue per thread
};

#endif