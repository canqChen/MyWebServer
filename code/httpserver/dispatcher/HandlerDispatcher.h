#ifndef HANDLERDISPATCHER_H
#define HANDLERDISPATCHER_H

#include<string>
#include<unordered_map>
#include<memory>

#include "../HttpCallbacks.h"
#include "../handler/HandlerChain.h"

using std::string_view;
class HandlerDispatcher: NoCopyable
{
public:
    // static HandlerDispatcher* getInstance();
    HandlerDispatcher();
    ~HandlerDispatcher();

    void registerHandlerCallback(string_view uri, string_view method, HandlerCallBack handler);
    void registerInterceptor(string_view uri, string_view method, InterceptorCallBack interceptor);
    HandlerCallBack getHandler(string_view uri, string_view method);
    // InterceptorCallBack getInterceptor(const std::string & uri, const std::string &method);
private:

    std::map<std::pair<std::string, std::string>, HandlerChain> handlerChains_;
};

#endif