#include "httpserver/dispatcher/HandlerDispatcher.h"
#include <regex>


void HandlerDispatcher::registerHandlerCallback(string_view uri, 
    string_view method, HandlerCallBack && handler)
{
    string uriStr(uri);
    string methodStr(method);
    std::pair<string, string> key(uriStr, methodStr);
    handlerChains_[key].addHandler(std::forward<HandlerCallBack>(handler));
}

void HandlerDispatcher::registerInterceptor(string_view uri, 
    string_view method, InterceptorCallBack && interceptor)
{
    string uriStr(uri);
    string methodStr(method);
    std::pair<string, string> key(uriStr, methodStr);
    handlerChains_[key].addInterceptor(std::forward<InterceptorCallBack>(interceptor));
}

HandlerCallBack HandlerDispatcher::getHandler(string_view uri, string_view method)
{
    string uriStr(uri);
    string methodStr(method);
    std::pair<string, string> key(uriStr, methodStr);
    if(handlerChains_.count(key)) {
        return handlerChains_[key];
    }
    for(auto&[key, hc] : handlerChains_) {
        std::regex pattern(key.first);
        std::smatch sm;
        if(methodStr == key.second && std::regex_match(uriStr, sm, pattern)) {
            return hc;
        }
    }
    return nullptr;
}

