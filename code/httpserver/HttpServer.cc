#include "httpserver/HttpServer.h"

#include "common/Log.h"
#include "httpserver/http/HttpCodec.h"
#include "httpserver/dispatcher/HandlerDispatcher.h"
#include "httpserver/handler/StaticResourceHandler.h"
#include "httpserver/interceptor/CheckHttpValidInterceptor.h"

HttpServer::HttpServer(const InetAddress& local, size_t nLoops, size_t nWorker) 
    : nLoops_(nLoops), nWorker_(nWorker), dispatcher_(new HandlerDispatcher()),
      tcpServerPool_(new TcpServerPool(local, nLoops_))
{
    assert(nLoops_ > 0 && nWorker_ > 0);
}

void HttpServer::start()
{
    __setHandlerCallback();
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


    auto handler = dispatcher_->getHandler(req->getRequestURI(), req->getRequestMethod());
    if(handler) {
        handler(req, resp);
    }
    else {
        resp->sendError(HttpStatus::NotFound404);
    }
    
    Buffer buff;
    codec->wrapHttp(resp, buff);
    conn->send(buff.readPtr(), buff.readableBytes());
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

void HttpServer::__setHandlerCallback() 
{
    // interceptor
    interceptorList_.emplace_back(std::move(std::make_unique<CheckHttpValidInterceptor>()));
    // static resource request
    handlerList_.emplace_back(std::move(std::make_unique<StaticResourceHandler>()));
    string staticPattern = "^.*\\.(css|js|eot|svg|ttf|woff|woff2|otf|html|htm|mp4|png|jpg|ico)$";
    dispatcher_->registerHandlerCallback(staticPattern, HttpMethod::GET, 
        std::bind(handlerList_.back()->doGet, handlerList_.back(), _1, _2));
    dispatcher_->registerInterceptor(staticPattern, HttpMethod::GET, 
        std::bind(interceptorList_.back()->doIntercept, interceptorList_.back(), _1, _2));
}