#include "TimerHeap.h"



// 向时间堆添加定时器节点，注册回调函数
void TimerHeap::AddNode(int fd, int timeout, const TimeoutCallBack& cb) {
    assert(fd >= 0);
    size_t idx;
    if(idx2Map.count(fd) == 0) {
        // 新节点，堆尾插入，调整堆(上浮即可)
        idx = heap.size();
        idx2Map[fd] = idx;
        heap.push_back({fd, Clock::now() + MS(timeout), cb});
        FloatUp(idx);
    }
    else {
        // 已有结点，更新超时时间，调整堆(可能节点在树的中间位置，要么上浮，要么下沉)
        idx = idx2Map[fd];
        heap[idx].expires = Clock::now() + MS(timeout);
        heap[idx].cb = cb;
        if(!SinkDown(idx, heap.size())) {
            FloatUp(idx);
        }
    }
}

// 触发回调函数，并删除指定fd结点
void TimerHeap::doWork(int fd) {
    if(heap.empty() || idx2Map.count(fd) == 0) {
        return;
    }
    size_t i = idx2Map[fd];
    TimerNode node = heap[i];
    node.cb();
    DeleteNode(i);
}

void TimerHeap::SwapNode(size_t i, size_t j) {
    assert(i >= 0 && i < heap.size());
    assert(j >= 0 && j < heap.size());
    std::swap(heap[i], heap[j]);
    idx2Map[heap[i].fd] = i;
    idx2Map[heap[j].fd] = j;
}

void TimerHeap::FloatUp(size_t index) {
    assert(index >= 0 && index < heap.size());
    int64_t root = (static_cast<int64_t>(index) - 1) / 2;        // 转有符号数，防止size_t下溢
    while(root >= 0) {
        if(heap[root] < heap[index])
            break; 
        SwapNode(index, root);
        index = root;
        root = (index - 1) / 2;
    }
}

bool TimerHeap::SinkDown(size_t index, size_t n) {
    assert(index >= 0 && index < heap.size());
    assert(n >= 0 && n <= heap.size());
    size_t root = index;
    size_t smallest = root * 2 + 1;
    while(smallest < n) {
        if(smallest + 1 < n && heap[smallest + 1] < heap[smallest]) 
            smallest++;
        if(heap[root] < heap[smallest])
            break;
        SwapNode(root, smallest);
        root = smallest;
        smallest = root * 2 + 1;
    }
    return root > index;
}

// 删除指定位置的结点
void TimerHeap::DeleteNode(size_t index){
    assert(!heap.empty() && index >= 0 && index < heap.size());
    // 将要删除的结点换到队尾，然后调整堆 
    size_t i = index;
    size_t n = heap.size() - 1;  // 节点数-1
    assert(i <= n);
    if(i < n) {         // 要删除节点不为尾节点，需要与队尾元素交换，然后上浮或者下沉调整
        SwapNode(i, n);
        if(!SinkDown(i, n)) {
            FloatUp(i);
        }
    }
    // 队尾元素删除
    idx2Map.erase(heap.back().fd);
    heap.pop_back();
}

// 更新fd对应节点超时时间
void TimerHeap::Update(int fd, int timeout) {
    //  调整指定fd的结点
    assert(!heap.empty() && idx2Map.count(fd) > 0);
    heap[idx2Map[fd]].expires = Clock::now() + MS(timeout);;
    SinkDown(idx2Map[fd], heap.size());   // 时间延长，下沉
}

// 心搏函数，清除超时结点
void TimerHeap::Tick() {
    if(heap.empty()) {
        return;
    }
    while(!heap.empty()) {
        TimerNode node = heap.front();    // 取得堆顶元素
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {    // 尚未超时，跳出
            break; 
        }
        node.cb();   // 超时回调
        Pop();     // 删除并调整
    }
}

void TimerHeap::Pop() {
    assert(!heap.empty());
    DeleteNode(0);
}

void TimerHeap::Clear() {
    idx2Map.clear();
    for(auto node:heap)     // 析构断开连接
        node.cb();
    heap.clear();
}

int TimerHeap::GetNextTick() {
    Tick();
    size_t res = -1;
    if(!heap.empty()) {
        res = std::chrono::duration_cast<MS>(heap.front().expires - Clock::now()).count();
        if(res < 0) { 
            res = 0; 
        }
    }
    return res;
}