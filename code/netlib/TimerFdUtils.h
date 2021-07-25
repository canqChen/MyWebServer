#ifndef TIMERFDUTILS_H
#define TIMERFDUTILS_H

#include <sys/timerfd.h>
#include <ratio> // std::nano::den
#include <unistd.h>
#include "../common/Log.h"

struct TimerFdUtils {
    int timerfdCreate() {
        // CLOCK_MONOTONIC 以绝对时间为准，获取的时间为系统重启到现在的时间，更改系统时间对其没有影响
        // TFD_CLOEXEC 为新的文件描述符设置运行时关闭标志
        // TFD_NONBLOCK 非阻塞io
        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (fd == -1) {
            LOG_FATAL("Fail in %s", "timer_create()");
        }
        return fd;
    }
    // timerfd可读回调
    void timerfdRead(int fd) {
        uint64_t val;
        ssize_t n = read(fd, &val, sizeof(val));
        if (n != sizeof(val)) {
            LOG_FATAL("timerfdRead get %ld, not %lu", n, sizeof(val));
        }
            
    }

    struct timespec durationFromNow(Timestamp when) {
        struct timespec ret;
        Nanosecond ns = when - clock::now();
        if (ns < 1ms) 
            ns = 1ms;

        ret.tv_sec = static_cast<time_t>(ns.count() / std::nano::den);
        ret.tv_nsec = ns.count() % std::nano::den;
        return ret;
    }

#if 0
struct itimerspec 
{
    struct timespec it_interval;   //间隔时间
    struct timespec it_value;      //第一次到期时间
};

struct timespec 
{
    time_t tv_sec;    //秒
    long tv_nsec;    //纳秒
};
int timerfd_settime(int fd, int flags, const struct itimerspec* new_value, struct itimerspec* old_value);
/*
flags 的值可以是 0 (相对时间), 可以是 TFD_TIMER_ABSTIME (绝对时间)
new_value 为定时器指定新设置, old_value 用来返回定时器的前一设置, 如果不关心, 可将其设置为 NULL
new_value： 指定新的超时时间，若 newValue.it_value非 0 则启动定时器，否则关闭定时器。
若 newValue.it_interval 为 0 则定时器只定时一次，否则之后每隔设定时间超时一次。
old_value：不为 NULL 时则返回定时器这次设置之前的超时时间
*/
#endif
    // 设置定时器超时时间，使when时间后唤醒epoller处理定时任务
    void timerfdSet(int fd, Timestamp when) {
        struct itimerspec oldtime, newtime;
        bzero(&oldtime, sizeof(itimerspec));
        bzero(&newtime, sizeof(itimerspec));
        newtime.it_value = durationFromNow(when);
        // 设置定时器超时时间
        int ret = timerfd_settime(fd, 0, &newtime, &oldtime);
        if (ret == -1) {
            LOG_ERROR("Fail in %s", "timerfd_settime()");
            exit(1);
        }
    }
};

#endif