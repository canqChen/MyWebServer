#include "netlib/TimerQueue.h"

#include <strings.h>

#include "netlib/TimerFdUtils.h"
#include "netlib/EventLoop.h"


TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(TimerFdUtils::timerfdCreate()),
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
    for (auto& p: timerSet_)
        delete p.second;
    ::close(timerfd_);
}

Timer* TimerQueue::addTimer(TimerCallback cb, Timestamp when, Nanosecond interval) {
    Timer* timer = new Timer(std::move(cb), when, interval);
    // 将修改定时器操作放到io线程中执行，保证线程安全
    loop_->runInLoop([=]() {
        auto ret = timerSet_.insert({when, timer});
        assert(ret.second);
        // 如果刚插入节点是最早到期的，更新最早到期时间
        if (timerSet_.begin() == ret.first) {
            TimerFdUtils::timerfdSet(timerfd_, when);
        }
    });
    return timer;
}

void TimerQueue::cancelTimer(Timer* timer) {
    loop_->runInLoop([timer, this](){
        timer->cancel();
        timerSet_.erase({timer->when(), timer});
        delete timer;
    });
}

// 定时器触发回调
void TimerQueue::__handleEventFdRead() {
    loop_->assertInLoopThread();

    TimerFdUtils::timerfdRead(timerfd_);

    Timestamp now(clock::now());
    for (auto& e: __getExpired(now)) {
        Timer* timer = e.second;
        assert(timer->isExpired(now));

        if (!timer->canceled())
            timer->run();
        if (!timer->canceled() && timer->isRepeat()) {
            timer->restart();
            e.first = timer->when();
            timerSet_.insert(e);
        }
        else {
            delete timer;
        }
    }
    // 处理完一次定时事件，设置下一次定时事件到期时间。关键
    if (!timerSet_.empty()) {
        TimerFdUtils::timerfdSet(timerfd_, timerSet_.begin()->first);
    }
}

std::vector<TimerQueue::Entry> TimerQueue::__getExpired(Timestamp now) {
    Entry en(now + 1ns, nullptr);
    std::vector<Entry> entries;

    auto end = timerSet_.lower_bound(en); // 返回第一个大于en的节点
    entries.assign(timerSet_.begin(), end);
    timerSet_.erase(timerSet_.begin(), end);

    return entries;
}

