#ifndef HANDLERCHAIN_H
#define HANDLERCHAIN_H

#include "../HttpCallbacks.h"
#include <vector>

using std::vector;

class HandlerChain
{
public:
    void operator()(const HttpRequestPtr& req, 
        HttpResponsePtr &resp)
    {
        for(auto &interceptor: interceptorList_) {
            if(!interceptor(req, resp)) {
                return;
            }
        }
        handler_(req, resp);
    }

    void addInterceptor(InterceptorCallBack &&interceptor) 
    {
        interceptorList_.emplace_back(std::forward<InterceptorCallBack>(interceptor));
    }

    void addHandler(HandlerCallBack &&handler) 
    {
        handler_ = std::forward<HandlerCallBack>(handler);
    }
private:
    vector<InterceptorCallBack> interceptorList_;
    HandlerCallBack handler_;
};

#endif