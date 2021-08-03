#include "HttpServer.h"

#include "../common/Log.h"
#include "http/HttpCodec.h"

#include "handler/StaticResourceHandler.h"
#include "interceptor/CheckHttpValidInterceptor.h"

HttpServer::HttpServer(const InetAddress& local, size_t nLoops, size_t nWorker) 
    : nLoops_(nLoops), nWorker_(nWorker), dispatcher_(new HandlerDispatcher()),
      tcpServerPool_(new TcpServerPool(local, nLoops_))
{
    assert(nLoops_ > 0 && nWorker_ > 0);
}

HttpServer::~HttpServer() 
{
    for(auto ptr : handlerList_) {
        delete ptr;
    }
    for(auto ptr : interceptorList_) {
        delete ptr;
    }
}

void HttpServer::start()
{
    __setHandlerCallback();
    tcpServerPool_->setMessageCallback(std::bind(&HttpServer::__onMessage, this, _1, _2));
    tcpServerPool_->setConnectionCallback(std::bind(&HttpServer::__onConnectionBuilt, this, _1));
    tcpServerPool_->setThreadInitCallback(std::bind(&HttpServer::__onThreadInit, this, _1));
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
    
    Buffer respBuff;
    codec->wrapHttp(resp, respBuff);
    conn->send(respBuff.readPtr(), respBuff.readableBytes());
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
    interceptorList_.emplace_back(new CheckHttpValidInterceptor());
    // static resource request
    handlerList_.emplace_back(new StaticResourceHandler());
    string staticPattern = "^.*\\.(css|js|eot|svg|ttf|woff|woff2|otf|html|htm|mp4|png|jpg|ico)$";
    dispatcher_->registerHandlerCallback(staticPattern, HttpMethod::GET, 
        [ptr = handlerList_.back()](const HttpRequestPtr &req, HttpResponsePtr & resp) {ptr->doGet(req, resp);});
    dispatcher_->registerInterceptor(staticPattern, HttpMethod::GET, 
        [ptr = interceptorList_.back()](const HttpRequestPtr &req, HttpResponsePtr & resp)->bool{return ptr->doIntercept(req, resp);});
}