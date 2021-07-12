// deprecated
#include "TimerHeap.h"

// 向时间堆添加定时器节点，注册回调函数
void TimerHeap::addNode(int fd, int timeout, const TimeoutCallBack& cb) {
    assert(fd >= 0);
    size_t idx;
    if(fd2Idx_.count(fd) == 0) {
        // 新节点，堆尾插入，调整堆(上浮即可)
        idx = heap_.size();
        fd2Idx_[fd] = idx;
        heap_.push_back({fd, Clock::now() + MS(timeout), cb});
        __siftUp(idx);
    }
    else {
        // 已有结点，更新超时时间，调整堆(可能节点在树的中间位置，要么上浮，要么下沉)
        idx = fd2Idx_[fd];
        heap_[idx].expires = Clock::now() + MS(timeout);
        heap_[idx].cb = cb;
        if(!__siftDown(idx, heap_.size())) {
            __siftUp(idx);
        }
    }
}

// 触发回调函数，并删除指定fd结点
void TimerHeap::doWork(int fd) {
    if(heap_.empty() || fd2Idx_.count(fd) == 0) {
        return;
    }
    size_t i = fd2Idx_[fd];
    TimerNode node = heap_[i];
    node.cb();
    __deleteNode(i);
}

void TimerHeap::__swapNode(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    fd2Idx_[heap_[i].fd] = i;
    fd2Idx_[heap_[j].fd] = j;
}

void TimerHeap::__siftUp(size_t index) {
    assert(index >= 0 && index < heap_.size());
    int64_t root = (static_cast<int64_t>(index) - 1) / 2;        // 转有符号数，防止size_t下溢
    while(root >= 0) {
        if(heap_[root] < heap_[index])
            break; 
        __swapNode(index, root);
        index = root;
        root = (index - 1) / 2;
    }
}

bool TimerHeap::__siftDown(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t root = index;
    size_t smallest = root * 2 + 1;
    while(smallest < n) {
        if(smallest + 1 < n && heap_[smallest + 1] < heap_[smallest]) 
            smallest++;
        if(heap_[root] < heap_[smallest])
            break;
        __swapNode(root, smallest);
        root = smallest;
        smallest = root * 2 + 1;
    }
    return root > index;
}

// 删除指定位置的结点
void TimerHeap::__deleteNode(size_t index){
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    // 将要删除的结点换到队尾，然后调整堆 
    size_t i = index;
    size_t n = heap_.size() - 1;  // 节点数-1
    assert(i <= n);
    if(i < n) {         // 要删除节点不为尾节点，需要与队尾元素交换，然后上浮或者下沉调整
        __swapNode(i, n);
        if(!__siftDown(i, n)) {
            __siftUp(i);
        }
    }
    // 队尾元素删除
    fd2Idx_.erase(heap_.back().fd);
    heap_.pop_back();
}

// 更新fd对应节点超时时间
void TimerHeap::update(int fd, int timeout) {
    //  调整指定fd的结点
    assert(!heap_.empty() && fd2Idx_.count(fd) > 0);
    heap_[fd2Idx_[fd]].expires = Clock::now() + MS(timeout);;
    __siftDown(fd2Idx_[fd], heap_.size());   // 时间延长，下沉
}

// 心搏函数，清除超时结点
void TimerHeap::tick() {
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();    // 取得堆顶元素
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {    // 尚未超时，跳出
            break; 
        }
        node.cb();   // 超时回调
        __pop();     // 删除并调整
    }
}

void TimerHeap::__pop() {
    assert(!heap_.empty());
    __deleteNode(0);
}

void TimerHeap::__clear() {
    fd2Idx_.clear();
    for(auto node:heap_)     // 析构断开连接
        node.cb();
    heap_.clear();
}

int TimerHeap::getNextTick() {
    tick();     // 清除无响应连接
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { 
            res = 0; 
        }
    }
    return res;
}