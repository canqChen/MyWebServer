#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "../netlib/TcpServerPool.h"

class HttpServer: NoCopyable
{
public:
    HttpServer(const InetAddress& local, size_t nLoops, size_t nWorker);
    ~HttpServer();

    void start();

private:
    void __onMessage(TcpConnectionPtr& conn, Buffer& buff);
    void __onThreadInit(size_t index);
    void __onConnectionBuilt(const TcpConnectionPtr& conn);
    void __setHandlerCallback();

private:
    size_t nLoops_, nWorker;
    std::unique_ptr<TcpServerPool> tcpServerPool_;
    std::unique_ptr<HandlerDispatcher> dispatcher_;
    std::vector<std::unique_ptr<AbstractHandler> > handlerList_;
    std::vector<std::unique_ptr<AbstractInterceptor> > interceptorList_;
};

#endif