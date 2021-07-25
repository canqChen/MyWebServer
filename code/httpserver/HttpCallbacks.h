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

// handler callback definition
typedef std::function<void(const std::unique_ptr<HttpRequest>&, 
        std::unique_ptr<HttpResponse> &)> HandlerCallBack;

typedef std::function<void(TcpConnectionPtr&, Buffer&)> MessageCallback;

#endif // HTTPCALLBACKS_H
