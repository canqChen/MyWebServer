#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <list>
#include "handler/AbstractHandler.h"
#include "interceptor/AbstractInterceptor.h"
#include "dispatcher/HandlerDispatcher.h"
#include "../netlib/TcpServer.h"

class HttpServer: NoCopyable
{
public:
    HttpServer(EventLoop *loop, const InetAddress& local);
    ~HttpServer();

    void start();

private:
    void __initHandlerCallback();
    void __onMessage(TcpConnectionPtr& conn, Buffer& buff);
    void __onConnection(const TcpConnectionPtr& conn);
    // delete idle connection periodically
    void __onTimer();

private:
    // std::unique_ptr<TcpServerPool> tcpServerPool_;

    std::unique_ptr<HandlerDispatcher> dispatcher_;
    std::vector<AbstractHandler*> handlerList_;
    std::vector<AbstractInterceptor*> interceptorList_;

    typedef std::unique_ptr<std::thread> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPtrList;
    typedef std::unique_ptr<TcpServer> TcpServerPtr;
    typedef std::vector<EventLoop*> SubEventLoopList;

    EventLoop* loop_;
    TcpServer server_;
    std::atomic_bool started_;
    std::atomic_size_t numConn_;
    const size_t maxConn_;
    const size_t keepAliveTime_; // seconds
    
private:
    typedef std::weak_ptr<TcpConnection> WeakConnPtr;
    // serializied operation, thread safe
    struct Entry {
        explicit Entry(const WeakConnPtr& wp) : wp_(wp)
        {}

        TcpConnectionPtr lock() {
            return wp_.lock();
        }

        ~Entry() {
            auto conn = wp_.lock();
            if(conn) {
                conn->shutdownWR();
            }
        }
        WeakConnPtr wp_;
    };
    typedef std::unique_ptr<Entry> EntryPtr;
    std::list<EntryPtr> connectionList_;
    std::unordered_map<string, std::list<EntryPtr>::iterator> connectionIndex_;
};

#endif