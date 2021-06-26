
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <sys/time.h>


// 阻塞队列， 生产者-消费者模型
template<class T>
class BlockQueue {
public:
    explicit BlockQueue(size_t MaxCapacity = 1000);

    ~BlockQueue();

    void Clear();

    bool Empty();

    bool Full();

    void Close();

    size_t Size();

    size_t Capacity();

    T& Front();

    void PushBack(const T &item);

    bool Pop(T &item);

    bool Pop(T &item, int timeout);

    void Flush();
private:
    std::queue<T> mQueue;

    size_t mCapacity;

    std::mutex mMtx;

    bool mClose;

    std::condition_variable mCondConsumer;

    std::condition_variable mCondProducer;
};


template<class T>
BlockQueue<T>::BlockQueue(size_t MaxCapacity) :mCapacity(MaxCapacity) {
    assert(MaxCapacity > 0);
    mClose = false;
}

template<class T>
BlockQueue<T>::~BlockQueue() {
    Close();
}

template<class T>
void BlockQueue<T>::Close() {
    {   
        std::lock_guard<std::mutex> locker(mMtx);
        std::queue<T> tmp;
        tmp.swap(mQueue);
        mClose = true;
    }
    mCondProducer.notify_all();
    mCondConsumer.notify_all();
}

// 唤醒消费者，写入日志
template<class T>
void BlockQueue<T>::Flush() {
    mCondConsumer.notify_all();
}

template<class T>
void BlockQueue<T>::Clear() {
    std::lock_guard<std::mutex> locker(mMtx);
    std::queue<T> tmp;
    tmp.swap(mQueue);
}

template<class T>
T& BlockQueue<T>::Front() {
    std::lock_guard<std::mutex> locker(mMtx);
    return mQueue.front();
}

template<class T>
size_t BlockQueue<T>::Size() {
    std::lock_guard<std::mutex> locker(mMtx);
    return mQueue.size();
}

template<class T>
size_t BlockQueue<T>::Capacity() {
    std::lock_guard<std::mutex> locker(mMtx);
    return mCapacity;
}

// 生产者-队尾插入
template<class T>
void BlockQueue<T>::PushBack(const T &item) {
    std::unique_lock<std::mutex> locker(mMtx);
    while(mQueue.size() >= mCapacity) {   // 队列满，等待
        mCondProducer.wait(locker);     // 释放锁，阻塞
    }
    mQueue.push(item);
    mCondConsumer.notify_one();
}

template<class T>
bool BlockQueue<T>::Empty() {
    std::lock_guard<std::mutex> locker(mMtx);
    return mQueue.empty();
}

template<class T>
bool BlockQueue<T>::Full(){
    std::lock_guard<std::mutex> locker(mMtx);
    return mQueue.size() >= mCapacity;
}

// 消费者-队首弹出，队列空则阻塞等待
template<class T>
bool BlockQueue<T>::Pop(T &item) {
    std::unique_lock<std::mutex> locker(mMtx);
    while(mQueue.empty()) {        // 队列空，等待生产
        mCondConsumer.wait(locker);
        if(mClose) {
            return false;
        }
    }
    item = mQueue.front();
    mQueue.pop();
    mCondProducer.notify_one();
    return true;
}

// 消费者-队首弹出，增加超时处理
template<class T>
bool BlockQueue<T>::Pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mMtx);
    while(mQueue.empty()){
        if(mCondConsumer.wait_for(locker, std::chrono::seconds(timeout)) 
                == std::cv_status::timeout){
            return false;
        }
        if(mClose){
            return false;
        }
    }
    item = mQueue.front();
    mQueue.pop();
    mCondProducer.notify_one();
    return true;
}

#endif // BLOCKQUEUE_H