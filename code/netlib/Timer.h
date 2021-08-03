#ifndef TIMER_H
#define TIMER_H

#include <cassert>

#include "Callbacks.h"
#include "Timestamp.h"
#include "../common/NoCopyable.h"

// 定时器类
class Timer: NoCopyable {
public:
    Timer(TimerCallback callback, Timestamp when, Nanosecond interval)
            : callback_(std::move(callback)),
              when_(when),
              interval_(interval),
              repeat_(interval_ > Nanosecond::zero()),
              canceled_(false)
    {
    }

    void run() 
    { 
        if (callback_) 
            callback_(); 
    }

    bool isRepeat() const 
    { 
        return repeat_; 
    }

    bool isExpired(Timestamp & now) const 
    { 
        return now >= when_; 
    }

    Timestamp when() const 
    { 
        return when_; 
    }

    void restart()
    {
        assert(repeat_);
        when_ += interval_;
    }

    void cancel()
    {
        assert(!canceled_);
        canceled_ = true;
    }

    bool canceled() const { 
        return canceled_; 
    }

private:
    TimerCallback callback_;    // 事件触发回调
    Timestamp when_;    // 触发事件时间戳
    const Nanosecond interval_; // 重复触发间隔
    bool repeat_;   // 是否重新触发
    bool canceled_; // 取消定时
};

#endif