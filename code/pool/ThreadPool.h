
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 12): _pool(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            // 创建线程池，并分离
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = _pool] {          // lambda表达式，初始化工作线程
                    std::unique_lock<std::mutex> locker(pool->mtx);  
                    while(true) {
                        if(!pool->tasks.empty()) {
                            // 取任务执行，有锁状态
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();   // 取完解锁，让池接收新任务，或者让其他线程处理新任务
                            task();
                            locker.lock();   // 为下次循环加锁，同时实现负载均衡
                        }
                        else if(pool->isClosed) 
                            break;
                        else
                            pool->cond.wait(locker);        // 无任务(空闲)，释放锁并阻塞，等待notify唤醒，唤醒自动获得锁
                    }
                }).detach();
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(_pool)) {
            {
                std::lock_guard<std::mutex> locker(_pool->mtx);  
                _pool->isClosed = true;         // isClosed = true，关闭所有线程
            }
            _pool->cond.notify_all();    // 唤醒所有线程，执行关闭
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(_pool->mtx);  // RAII，创建即加锁，作用域技术析构解锁
            _pool->tasks.emplace(std::forward<F>(task));
        }
        _pool->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx;          // 任务队列读写锁
        std::condition_variable cond;    // 条件变量，记录任务队列是否有任务(任务数量)
        bool isClosed;      // 关闭与否
        std::queue<std::function<void()> > tasks;   // 任务队列
    };
    std::shared_ptr<Pool> _pool;
};


#endif //THREADPOOL_H