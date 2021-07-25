#include "./HttpServer.h"
#include "../common/Log.h"
#include "./http/HttpCodec.h"
#include "./dispatcher/HandlerDispatcher.h"


HttpServer::HttpServer(const InetAddress& local, size_t nLoops, size_t nWorker) 
    : nLoops_(nLoops), nWorker_(nWorker), tcpServerPool_(local, nLoops_)
{
    assert(nLoops_ > 0 && nWorker_ > 0);
}

void HttpServer::start() 
{
    tcpServerPool_->setMessageCallback(std::bind(__onMessage, this, _1, _2));
    tcpServerPool_->setConnectionCallback(std::bind(__onConnectionBuilt, this, _1));
    tcpServerPool_->setThreadInitCallback(std::bind(__onThreadInit, this, _1));
    tcpServerPool_->start();
}

void HttpServer::__onMessage(TcpConnectionPtr& conn, Buffer& buff)
{
    auto codec = std::make_unique<HttpCodec>();
    auto req = std::move(codec->parseHttp(buff));
    if(req == nullptr) {
        return;
    }
    auto resp = std::make_unique<HttpResponse>();
    auto handler = HandlerDispatcher::getInstance()->getHandler(req->getRequestURI(), req->getRequestMethod());
    handler(req, resp);
    auto respMessage = codec->wrapHttp(conn, resp);
    conn->send(respMessage);
}

void HttpServer::__onThreadInit(size_t index) 
{
    LOG_INFO("EventLoop thread #%lu started", index);
}

void HttpServer::__onConnectionBuilt(const TcpConnectionPtr& conn) 
{
    LOG_INFO("connection %s -> %s %s",
         conn->peer().toIpPort().c_str(),
         conn->local().toIpPort().c_str(),
         conn->isConnected() ? "up" : "down");
}