#include <strings.h>


#include "./TimerFdUtils.h"
#include "./EventLoop.h"
#include "./TimerQueue.h"


TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(timerfdCreate()),
          timerChannel_(loop, timerfd_) {
    // 每个loop都有各自的定时器队列
    loop_->assertInLoopThread();
    // 注册定时器事件回调
    timerChannel_.setReadCallback([this](){ 
        __handleEventFdRead(); 
    });
    // 注册监听timerfd可读事件
    timerChannel_.enableRead();
}

TimerQueue::~TimerQueue() {
    for (auto& p: timers_)
        delete p.second;
    ::close(timerfd_);
}

Timer* TimerQueue::addTimer(TimerCallback cb, Timestamp when, Nanosecond interval) {
    Timer* timer = new Timer(std::move(cb), when, interval);
    // 将修改定时器操作放到io线程中，保证线程安全
    loop_->runInLoop([=](){
        auto ret = timers_.insert({when, timer});
        assert(ret.second);
        // 如果刚插入节点是最早到期的，更新最早到期时间
        if (timers_.begin() == ret.first) {
            timerfdSet(timerfd_, when);
        }
    });
    return timer;
}

void TimerQueue::cancelTimer(Timer* timer) {
    loop_->runInLoop([timer, this](){
        timer->cancel();
        timers_.erase({timer->when(), timer});
        delete timer;
    });
}
// 定时器触发回调
void TimerQueue::__handleEventFdRead() {
    loop_->assertInLoopThread();
    timerfdRead(timerfd_);

    Timestamp now(clock::now());
    for (auto& e: __getExpired(now)) {
        Timer* timer = e.second;
        assert(timer->expired(now));

        if (!timer->canceled())
            timer->run();
        if (!timer->canceled() && timer->repeat()) {
            timer->restart();
            e.first = timer->when();
            timers_.insert(e);
        }
        else {
            delete timer;
        }
    }
    // 处理完一次定时事件，设置下一次定时事件为最早到期时间。重要！
    if (!timers_.empty()) {
        timerfdSet(timerfd_, timers_.begin()->first);
    }
}

std::vector<TimerQueue::Entry> TimerQueue::__getExpired(Timestamp now) {
    Entry en(now + 1ns, nullptr);
    std::vector<Entry> entries;

    auto end = timers_.lower_bound(en); // 返回第一个大于en的节点
    entries.assign(timers_.begin(), end);
    timers_.erase(timers_.begin(), end);

    return entries;
}

