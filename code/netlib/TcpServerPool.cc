#include "../common/Log.h"
#include "./TcpConnection.h"

#include "./EventLoop.h"
#include "./TcpServerPool.h"

// set upper-level logic callback while construct tcp server
TcpServerPool::TcpServerPool(const InetAddress& local, size_t nThread)
        : baseLoop_(new EventLoop()), numThreads_(nThread),
          started_(false), local_(local),
          threadInitCallback_(defaultThreadInitCallback),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(MessageCallback())
{
    // n == 0 || n == 1: all things run in baseLoop thread
    // n > 1: set another (n - 1) eventLoop threads.
    eventLoops_.resize(n - 1);
    LOG_DEBUG("Create TcpServerPool() %s", local.toIpPort().c_str());
}

TcpServerPool::~TcpServerPool()
{
    for (auto& loop: eventLoops_) {
        if (loop != nullptr)
            loop->quit();
    }
    for (auto& thread: threads_) {
        thread->join();
    }
    LOG_TRACE("~TcpServerPool()");
}

// void TcpServerPool::setNumThread(size_t n) 
// {
//     baseLoop_->assertInLoopThread();
//     assert(n > 0);
//     assert(!started_);
//     numThreads_ = n;
//     eventLoops_.resize(n);
// }

void TcpServerPool::start()
{
    if (started_.exchange(true)) {
        return;     // already started
    }

    baseLoop_->runInLoop([=](){__startInLoop();});
    baseLoop_->loop();
}

void TcpServerPool::__startInLoop()
{
    LOG_INFO("TcpServerPool::start() %s with %lu eventLoop thread(s)",
         local_.toIpPort().c_str(), numThreads_);

    // start main event loop (loop in main function)
    baseServer_ = std::make_unique<TcpServer>(baseLoop_, local_);
    baseServer_->setConnectionCallback(connectionCallback_);
    baseServer_->setMessageCallback(messageCallback_);
    baseServer_->setWriteCompleteCallback(writeCompleteCallback_);
    threadInitCallback_(0);
    baseServer_->startListen();

    // start sub tcp servers and sub event loops in threads
    for (size_t i = 1; i < numThreads_; ++i) {
        auto thread = new std::thread(std::bind(
                &TcpServerPool::__runInThread, this, i));
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (eventLoops_[i] == nullptr) {
                cond_.wait(lock);
            }
        }
        threads_.emplace_back(thread);
    }
}

// create, start sub tcp servers and sub event loops in threads
void TcpServerPool::__runInThread(size_t index) 
{
    EventLoop loop;
    TcpServer server(&loop, local_);

    server.setConnectionCallback(connectionCallback_);
    server.setMessageCallback(messageCallback_);
    server.setWriteCompleteCallback(writeCompleteCallback_);

    {
        std::lock_guard<std::mutex> guard(mutex_);
        eventLoops_[index] = &loop;
        cond_.notify_one();
    }

    threadInitCallback_(index);
    server.startListen();  // start listen
    loop.loop();
    // quit
    eventLoops_[index] = nullptr;
}
