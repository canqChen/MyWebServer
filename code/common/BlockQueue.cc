#include "BlockQueue.h"

template<class T>
BlockQueue<T>::BlockQueue(size_t MaxCapacity) :capacity_(MaxCapacity) {
    assert(capacity_ > 0);
    isClose_ = false;
}

template<class T>
BlockQueue<T>::~BlockQueue() {
    close();
}

template<class T>
void BlockQueue<T>::close() {
    {   
        std::lock_guard<std::mutex> locker(mtx_);
        std::queue<T> tmp;
        tmp.swap(queue_);
        isClose_ = true;
    }
    producerCond_.notify_all();
    consumerCond_.notify_all();
}

// 唤醒消费者
template<class T>
void BlockQueue<T>::flush() {
    std::lock_guard<std::mutex> locker(mtx_);
    consumerCond_.notify_one();
}

// 清空队列，只用于日志系统
template<class T>
void BlockQueue<T>::flushAll(FILE * fp) {
    std::lock_guard<std::mutex> locker(mtx_);
    while(!queue_.empty()) {
        T str = queue_.front();
        queue_.pop();
        fputs(str.c_str(), fp);
    }
}

template<class T>
void BlockQueue<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    std::queue<T> tmp;
    tmp.swap(queue_);
}

template<class T>
T& BlockQueue<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return queue_.front();
}

template<class T>
size_t BlockQueue<T>::size() const {
    std::lock_guard<std::mutex> locker(mtx_);
    return queue_.size();
}

template<class T>
size_t BlockQueue<T>::capacity() const{
    return capacity_;
}

// 生产者-队尾插入
template<class T>
void BlockQueue<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(queue_.size() >= capacity_) {   // 队列满，等待
        producerCond_.wait(locker);     // 释放锁，阻塞
    }
    queue_.push(item);
    locker.unlock();
    consumerCond_.notify_one();
}

template<class T>
bool BlockQueue<T>::empty() const{
    std::lock_guard<std::mutex> locker(mtx_);
    return queue_.empty();
}

template<class T>
bool BlockQueue<T>::full() const{
    std::lock_guard<std::mutex> locker(mtx_);
    return queue_.size() >= capacity_;
}

// 消费者-队首弹出，队列空则阻塞等待
template<class T>
bool BlockQueue<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(queue_.empty()) {        // 队列空，等待生产
        consumerCond_.wait(locker);
        if(isClose_) {
            return false;
        }
    }
    item = queue_.front();
    queue_.pop();
    locker.unlock();
    producerCond_.notify_one();
    return true;
}

// 消费者-队首弹出，增加超时处理
template<class T>
bool BlockQueue<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(queue_.empty()){
        if(consumerCond_.wait_for(locker, std::chrono::seconds(timeout)) 
                == std::cv_status::timeout){
            return false;
        }
        if(isClose_) {
            return false;
        }
    }
    item = queue_.front();
    queue_.pop();
    locker.unlock();
    producerCond_.notify_one();
    return true;
}