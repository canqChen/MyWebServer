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

private:
    typedef std::unique_ptr<TcpServerPool> TcpServerPoolPtr;
    size_t nLoops_, nWorker;
    TcpServerPoolPtr tcpServerPool_;

};

#endif