#ifndef CHECKHTTPVALIDINTERCEPTOR_H
#define CHECKHTTPVALIDINTERCEPTOR_H

#include "httpserver/interceptor/AbstractInterceptor.h"

class CheckHttpValidInterceptor: public AbstractInterceptor
{
public:
    bool doIntercept(const HttpRequestPtr &, HttpResponsePtr&);
};

#endif