#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "Channel.h"

#include <memory>
#include <set>

#include "Timer.h"

class EventLoop;


// 定时器队列，底层rb-tree(set)实现
class TimerQueue: NoCopyable {
public:
    explicit
    TimerQueue(EventLoop* loop);
    ~TimerQueue();
    
    Timer* addTimer(TimerCallback cb, Timestamp when, Nanosecond interval);
    void cancelTimer(Timer* timer);

private:
    typedef std::unique_ptr<Timer> TimerPtr;
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;

    void __handleEventFdRead();
    std::vector<Entry> __getExpired(Timestamp now);

private:
    EventLoop* loop_;   // 定时器队列所属loop
    const int timerfd_; // 定时器fd
    Channel timerChannel_;  // 定时器fd对应channel
    TimerList timerSet_;
};

#endif //TIMERQUEUE_H
