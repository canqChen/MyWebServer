
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

    void clear();

    bool empty() const;

    bool full() const;

    void close();

    size_t size() const;

    size_t capacity() const;

    T& front();

    void push_back(const T &item);

    bool pop(T &item);

    bool pop(T &item, int timeout);

    void flush();
private:
    std::queue<T> queue_;

    size_t capacity_;

    std::mutex mtx_;

    bool isClose_;

    std::condition_variable consumerCond_;

    std::condition_variable producerCond_;
};

#endif // BLOCKQUEUE_H