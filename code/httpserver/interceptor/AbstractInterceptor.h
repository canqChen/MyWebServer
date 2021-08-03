#ifndef ABSTRACTINTERCEPTOR_H
#define ABSTRACTINTERCEPTOR_H

#include "httpserver/HttpCallbacks.h"

class AbstractInterceptor
{
public:
    virtual bool doIntercept(const HttpRequestPtr &, HttpResponsePtr&) = 0;
};

#endif // ABSTRACTINTERCEPTOR_H