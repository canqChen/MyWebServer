#include "HttpServerPool.h"

#include "../common/Log.h"

HttpServerPool::HttpServerPool(const InetAddress& local, size_t nServer) 
    : local_(local), nServer_(nServer), started_(false)
{
    assert(nServer_ > 0);
    // n == 0 || n == 1: all things run in baseLoop thread
    // n > 1: set another (n - 1) eventLoop threads.
    eventLoopList_.resize(nServer_ - 1);
    LOG_INFO("Create HttpServerPool on %s", local.toIpPort().c_str());
}

HttpServerPool::~HttpServerPool() 
{
    for (auto& loop: eventLoopList_) {
        if (loop != nullptr)
            loop->quit();
    }
    for (auto& thread: threads_) {
        thread->join();
    }
    LOG_TRACE("~HttpServerPool()");
}


void HttpServerPool::__onThreadInit(size_t index) 
{
    LOG_INFO("EventLoop thread #%lu started", index);
}


void HttpServerPool::start()
{
    if (started_.exchange(true)) {
        return;     // already started
    }

    EventLoop baseLoop;
    HttpServer baseServer(&baseLoop, local_);

    baseLoop.runInLoop([=](){__startInLoop();});

    // start main event loop (loop in main function)
    baseServer.start();
}

void HttpServerPool::__startInLoop()
{
    LOG_INFO("HttpServerPool::start() %s with %lu eventLoop thread(s)",
         local_.toIpPort().c_str(), nServer_);

    __onThreadInit(0);

    // start sub tcp servers and sub event loops in threads
    for (size_t i = 1; i < nServer_; ++i) {
        auto thread = new std::thread(std::bind(
                &HttpServerPool::__runInThread, this, i));
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (eventLoopList_[i] == nullptr) {
                cond_.wait(lock);
            }
        }
        threads_.emplace_back(thread);
    }
}

// create, start sub tcp servers and sub event loops in threads
void HttpServerPool::__runInThread(size_t index) 
{
    EventLoop loop;
    HttpServer server(&loop, local_);

    {
        std::lock_guard<std::mutex> guard(mutex_);
        eventLoopList_[index] = &loop;
        cond_.notify_one();
    }
    __onThreadInit(index);

    server.start();
    // quit
    eventLoopList_[index] = nullptr;
}