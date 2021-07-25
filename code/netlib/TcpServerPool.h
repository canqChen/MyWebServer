#ifndef TCPSERVERPOOL_H
#define TCPSERVERPOOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "TcpServerSingle.h"
#include "./InetAddress.h"
#include "./Callbacks.h"
#include "../common/NoCopyable.h"

class EventLoopThread;
class TcpServer;
class EventLoop;
class InetAddress;

// create multiple sub server, one loop in each sub server
class TcpServerPool: NoCopyable
{
public:
    TcpServerPool(const InetAddress& local, size_t nThread);
    ~TcpServerPool();
    // n == 0 || n == 1: all things run in baseLoop thread
    // n > 1: set another (n - 1) eventLoop threads.
    // void setNumThread(size_t n);
    
    // set all threads begin to loop and accept new connections
    // except the baseLoop thread
    void start();

    void setThreadInitCallback(const ThreadInitCallback& cb) 
    { 
        threadInitCallback_ = cb; 
    }

    void setConnectionCallback(const ConnectionCallback& cb) 
    { 
        connectionCallback_ = cb; 
    }

    void setMessageCallback(const MessageCallback& cb)
    { 
        messageCallback_ = cb; 
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) 
    { 
        writeCompleteCallback_ = cb;
    }

private:
    void __startInLoop();
    void __runInThread(size_t index);

    typedef std::unique_ptr<std::thread> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPtrList;
    typedef std::unique_ptr<TcpServer> TcpServerPtr;
    typedef std::vector<EventLoop*> SubEventLoopList;

    std::unique_ptr<EventLoop> baseLoop_;
    TcpServerPtr baseServer_;
    ThreadPtrList threads_;
    SubEventLoopList eventLoops_; // how many sub event loops
    size_t numThreads_;     // how many thread, one loop in each thread
    std::atomic_bool started_;
    InetAddress local_;     // address to listen
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback threadInitCallback_;  // log
    ConnectionCallback connectionCallback_; // log connection info while new connection was built or closed
    MessageCallback messageCallback_;   // upper-level logic
    WriteCompleteCallback writeCompleteCallback_;
};


#endif // TCPSERVERPOOL_H
