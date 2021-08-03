#ifndef ABSTRACTINTERCEPTOR_H
#define ABSTRACTINTERCEPTOR_H

#include "../HttpCallbacks.h"

class AbstractInterceptor
{
public:
    virtual bool doIntercept(const HttpRequestPtr &, HttpResponsePtr&) = 0;
};

#endif // ABSTRACTINTERCEPTOR_H