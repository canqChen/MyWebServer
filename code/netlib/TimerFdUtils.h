#ifndef TIMERFDUTILS_H
#define TIMERFDUTILS_H

#include <sys/timerfd.h>
#include <ratio> // std::nano::den
#include <unistd.h>
#include "../common/Log.h"
#include "Timestamp.h"


struct TimerFdUtils {
    static int timerfdCreate();
    // timerfd可读回调
    static void timerfdRead(int fd);

    static struct timespec durationFromNow(Timestamp when);

    // 设置定时器超时时间，使when时间后唤醒epoller处理定时任务
    static void timerfdSet(int fd, Timestamp when);

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
};

#endif