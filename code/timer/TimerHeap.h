
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/Log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;   // C++11 高精度时钟，纳秒精度
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;



class TimerHeap {
public:
    TimerHeap() { heap_.reserve(64); }   // 预留至少64个空间

    ~TimerHeap() { __clear(); }
    
    void update(int fd, int newExpires);

    void addNode(int fd, int timeOut, const TimeoutCallBack& cb);

    void doWork(int fd);

    void tick();

    int getNextTick();

private:
    struct TimerNode {
        int fd;         // 定时器对应fd
        TimeStamp expires;        //  超时时间
        TimeoutCallBack cb;         // 超时回调函数
        bool operator < (const TimerNode& t) {
            return expires < t.expires;
        }
    };
    void __clear();

    void __pop();

    void __deleteNode(size_t i);
    
    void siftUp(size_t index);

    bool __siftDown(size_t index, size_t n);

    void __swapNode(size_t i, size_t j);

    std::vector<TimerNode> heap_;    // 小根堆数据结构

    std::unordered_map<int, size_t> fd2Idx_;         // hash表，建立fd->heap下标的映射
};

#endif //HEAP_TIMER_H