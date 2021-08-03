
#include <cassert>
#include <sys/eventfd.h>
#include <sys/types.h> // pid_t
#include <unistd.h>	// syscall()
#include <syscall.h> // SYS_gettid
#include <signal.h>
#include <numeric>


#include "../common/Log.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"


// __thread声明每个线程独立变量
__thread EventLoop* t_Eventloop = nullptr;

pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

class IgnoreSigPipe {
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe ignore;

EventLoop::EventLoop()
        : tid_(gettid()), quit_(false),
          doingPendingTasks_(false), poller_(this),
          wakeupFd_(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
          wakeupChannel_(this, wakeupFd_),
          timerQueue_(this) 
{
    if (wakeupFd_ == -1)
        LOG_FATAL("EventLoop::eventfd() create fail");

    wakeupChannel_.setReadCallback([this](){__handleWakeUpFdRead();});
    wakeupChannel_.enableRead();

    assert(t_Eventloop == nullptr);
    t_Eventloop = this;
}

EventLoop::~EventLoop() 
{
    assert(t_Eventloop == this);
    t_Eventloop = nullptr;
}

void EventLoop::loop() 
{
    assertInLoopThread();
    LOG_DEBUG("EventLoop %p polling", this);
    quit_ = false;
    while (!quit_) {
        activeChannels_.clear();
        poller_.poll(activeChannels_);
        for (auto channel: activeChannels_) {
            channel->handleEvents();
        }
        // deal tasks call from other thread
        __doPendingTasks();
    }
    LOG_DEBUG("EventLoop %p quit", this);
}

void EventLoop::quit() 
{
    assert(!quit_);
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(const Task& task) 
{
    if (isInLoopThread()) {
        task();
    }
    else {
        queueInLoop(task);
    }
}

// run task immediately if in thread or enqueue if in other thread
void EventLoop::runInLoop(Task&& task) 
{
    if (isInLoopThread()) {
        task();
    }
    else {
        queueInLoop(std::move(task));
    }
}

void EventLoop::queueInLoop(const Task& task) 
{
    {
        std::lock_guard<std::mutex> guard(mutex_);
        pendingTasks_.push_back(task);
    }
    // if we are not in loop thread, just wake up loop thread to handle new task
    // if we are in loop thread && doing pending task, wake up too.
    // note that the following code has race condition:
    //     if (doingPendingTasks_ || isInLoopThread())
    if (!isInLoopThread() || doingPendingTasks_) {
        wakeup();
    }
}

void EventLoop::queueInLoop(Task&& task) 
{
    {
        std::lock_guard<std::mutex> guard(mutex_);
        pendingTasks_.push_back(std::move(task));
    }
    if (!isInLoopThread() || doingPendingTasks_)
        wakeup();
}

Timer* EventLoop::runAt(Timestamp when, TimerCallback callback) 
{
    return timerQueue_.addTimer(std::move(callback), when, Millisecond::zero());
}

Timer* EventLoop::runAfter(Nanosecond interval, TimerCallback callback) 
{
    return runAt(clock::now() + interval, std::move(callback));
}

Timer* EventLoop::runEvery(Nanosecond interval, TimerCallback callback) 
{
    return timerQueue_.addTimer(std::move(callback),
                                clock::now() + interval,
                                interval);
}

void EventLoop::cancelTimer(Timer* timer) 
{
    timerQueue_.cancelTimer(timer);
}

void EventLoop::wakeup() 
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)){
        LOG_FATAL("EventLoop::wakeup() should ::write() %lu bytes", sizeof(one));
    }
}

void EventLoop::updateChannel(Channel* channel) 
{
    assertInLoopThread();
    poller_.updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) 
{
    assertInLoopThread();
    channel->disableAll();
}

void EventLoop::assertInLoopThread() 
{
    assert(isInLoopThread());
}

void EventLoop::assertNotInLoopThread() 
{
    assert(!isInLoopThread());
}

bool EventLoop::isInLoopThread() 
{
    // tid_ is constant, don't worry about thread safety
    return tid_ == gettid();
}

void EventLoop::__doPendingTasks() 
{
    assertInLoopThread();
    std::vector<Task> tasks;

    {
        // shorten the critical area by a single swap
        std::lock_guard<std::mutex> guard(mutex_);
        tasks.swap(pendingTasks_);
    }

    doingPendingTasks_ = true;
    for (Task& task: tasks) {
        task();
    }
    doingPendingTasks_ = false;
}

void EventLoop::__handleWakeUpFdRead() 
{
    uint64_t one;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_FATAL("EventLoop::__handleWakeUpFdRead() should ::read() %lu bytes", sizeof(one));
    }
}