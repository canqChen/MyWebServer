#ifndef HTTPSERVERPOOL_H
#define HTTPSERVERPOOL_H

#include "HttpServer.h"

class HttpServerPool: NoCopyable
{
public:
    HttpServerPool(const InetAddress& local, size_t nServer);
    ~HttpServerPool();

    void start();

private:
    void __onThreadInit(size_t index);

private:
    size_t nServer_;

    void __startInLoop();
    void __runInThread(size_t index);

    typedef std::unique_ptr<std::thread> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPtrList;
    typedef std::unique_ptr<HttpServer> HttpServerPtr;
    typedef std::vector<EventLoop*> SubEventLoopList;

    ThreadPtrList threads_;
    SubEventLoopList eventLoopList_; // how many sub event loops
    std::atomic_bool started_;
    InetAddress local_;     // address to listen
    std::mutex mutex_;  // protect 
    std::condition_variable cond_;
};

#endif