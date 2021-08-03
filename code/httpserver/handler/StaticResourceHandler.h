#ifndef STATICRESOURCEHANDLER_H
#define STATICRESOURCEHANDLER_H

#include "httpserver/handler/AbstractHandler.h"

class StaticResourceHandler : public AbstractHandler
{
public:
    virtual void doGet(const HttpRequestPtr &req, HttpResponsePtr & resp);

    virtual void doPost(const HttpRequestPtr &req, HttpResponsePtr & resp) {}
};


#endif