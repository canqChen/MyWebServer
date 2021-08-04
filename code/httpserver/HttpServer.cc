#include "HttpServer.h"
#include "../netlib/Timestamp.h"
#include "../common/Log.h"
#include "http/HttpCodec.h"

#include "handler/StaticResourceHandler.h"
#include "interceptor/CheckHttpValidInterceptor.h"

HttpServer::HttpServer(EventLoop* loop, const InetAddress& local) 
    : loop_(loop), server_(loop, local), dispatcher_(new HandlerDispatcher()), 
      numConn_(0), maxConn_(Config::MAXCONN), keepAliveTime_(Config::KEEPALIVE_TIME),
      started_(false)
{
    LOG_INFO("Create HttpServer on %s", local.toIpPort().c_str());
    __initHandlerCallback();
    server_.setConnectionCallback(std::bind(&HttpServer::__onConnection, this, _1));
    server_.setMessageCallback(std::bind(&HttpServer::__onMessage, this, _1, _2));
    // check idle connections every 2 seconds
    loop->runEvery(2s, std::bind(&HttpServer::__onTimer, this));
}

HttpServer::~HttpServer() 
{
    for(auto ptr : handlerList_) {
        delete ptr;
    }
    for(auto ptr : interceptorList_) {
        delete ptr;
    }
    LOG_TRACE("~HttpServer()");
}

void HttpServer::__initHandlerCallback() 
{
    // interceptor
    interceptorList_.emplace_back(new CheckHttpValidInterceptor());
    // static resource request
    handlerList_.emplace_back(new StaticResourceHandler());
    string staticPattern = "(^/$|^.*\\.(css|js|eot|svg|ttf|woff|woff2|otf|html|htm|mp4|png|jpg|ico)$|^/.+$)";
    dispatcher_->registerHandlerCallback(staticPattern, HttpMethod::GET, 
        [ptr = handlerList_.back()](const HttpRequestPtr &req, HttpResponsePtr & resp) {ptr->doGet(req, resp);});
    dispatcher_->registerInterceptor(staticPattern, HttpMethod::GET, 
        [ptr = interceptorList_.back()](const HttpRequestPtr &req, HttpResponsePtr & resp)->bool{return ptr->doIntercept(req, resp);});
}

void HttpServer::__onMessage(TcpConnectionPtr& conn, Buffer& buff)
{
    // active conneciton, move the connection to the list end
    conn->setExpireTime(GetTimestamp::nowAfter(Second(keepAliveTime_)));
    connectionList_.splice(connectionList_.end(), connectionList_, connectionIndex_[conn->name()]);

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

    if(!req->isKeepAlive()) {
        LOG_DEBUG("Not keep-alive, delete connection: %s", conn->name().c_str());
        conn->setWriteCompleteCallback([](const TcpConnectionPtr& c){c->shutdownWR();});
    }
    conn->send(respBuff.readPtr(), respBuff.readableBytes());
}

void HttpServer::__onConnection(const TcpConnectionPtr& conn) 
{
    LOG_INFO("connection %s -> %s %s",
         conn->peer().toIpPort().c_str(),
         conn->local().toIpPort().c_str(),
         conn->isConnected() ? "up" : "down");

    if(conn->isConnected()) {
        ++numConn_;
        if(numConn_ > maxConn_) {
            LOG_INFO("Reach max connection %ld, force close", maxConn_);
            conn->forceClose();
        }
        conn->setExpireTime(GetTimestamp::nowAfter(Second(keepAliveTime_)));
        auto ptr = std::make_unique<Entry>(conn);
        connectionList_.push_back(std::move(ptr));
        connectionIndex_.insert({conn->name(), --connectionList_.end()});
    }
    else {
        --numConn_;
    }
    
}

void HttpServer::__onTimer() 
{
    LOG_DEBUG("Current conneciton list size: %d", int(connectionList_.size()));
    auto now = GetTimestamp::now();
    for(auto it = connectionList_.begin(); it != connectionList_.end();) {
        auto ptr = (*it)->lock();
        if(ptr) {
            if(ptr->getExpireTime() <= now) {
                LOG_DEBUG("Delete idle connection %s:", ptr->name().c_str());
                connectionIndex_.erase(ptr->name());
                connectionList_.erase(it++);
            } else {
                break;
            }
        }
        else {
            connectionList_.erase(it++);
        }
    }
}

void HttpServer::start()
{
    loop_->assertInLoopThread();
    if (started_.exchange(true)) {
        return;     // already started
    }
    server_.startListen();
    loop_->loop();
}
