#ifndef ABSTRACTHANDLER_H
#define ABSTRACTHANDLER_H

#include "httpserver/HttpCallbacks.h"

class AbstractHandler
{
public:
    virtual void doGet(const HttpRequestPtr &, HttpResponsePtr&) = 0;
    virtual void doPost(const HttpRequestPtr &, HttpResponsePtr&) = 0;
};

#endif