#ifndef HTTPCALLBACKS_H
#define HTTPCALLBACKS_H

#include <memory>
#include <functional>

#include "../netlib/TcpConnection.h"

using namespace std::string_view_literals;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

class HttpRequest;
class HttpResponse;
class Buffer;

typedef std::unique_ptr<HttpRequest> HttpRequestPtr;
typedef std::unique_ptr<HttpResponse> HttpResponsePtr;

// handler callback definition
typedef std::function<void(const HttpRequestPtr&, 
        HttpResponsePtr &)> HandlerCallBack;

typedef std::function<bool(const HttpRequestPtr&, 
        HttpResponsePtr &)> InterceptorCallBack;

typedef std::function<void(TcpConnectionPtr&, Buffer&)> MessageCallback;

#endif // HTTPCALLBACKS_H
