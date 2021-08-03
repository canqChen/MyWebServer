#include "./TimerFdUtils.h"

int TimerFdUtils::timerfdCreate() {
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
void TimerFdUtils::timerfdRead(int fd) {
    uint64_t val;
    ssize_t n = read(fd, &val, sizeof(val));
    if (n != sizeof(val)) {
        LOG_FATAL("timerfdRead get %ld, not %lu", n, sizeof(val));
    }
}

struct timespec TimerFdUtils::durationFromNow(Timestamp when) {
    struct timespec ret;
    Nanosecond ns = when - clock::now();
    if (ns < 1ms) 
        ns = 1ms;

    ret.tv_sec = static_cast<time_t>(ns.count() / std::nano::den);
    ret.tv_nsec = ns.count() % std::nano::den;
    return ret;
}

// 设置定时器超时时间，使when时间后唤醒epoller处理定时任务
void TimerFdUtils::timerfdSet(int fd, Timestamp when) {
    struct itimerspec oldtime, newtime;
    bzero(&oldtime, sizeof(itimerspec));
    bzero(&newtime, sizeof(itimerspec));
    newtime.it_value = durationFromNow(when);
    // 设置定时器超时时间
    int ret = timerfd_settime(fd, 0, &newtime, &oldtime);
    if (ret == -1) {
        LOG_FATAL("Fail in timerfd_settime()");
    }
}