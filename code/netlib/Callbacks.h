#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <memory>
#include <functional>

using namespace std::string_view_literals;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

class Buffer;
class TcpConnection;
class InetAddress;

// tcpconnection callback definition
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef std::function<void(TcpConnectionPtr&, Buffer&)> MessageCallback;

// channel callback definition
typedef std::function<void()> ErrorCallback;
typedef std::function<void()> ReadCallback;
typedef std::function<void()> WriteCallback;
typedef std::function<void()> CloseCallback;

typedef std::function<void(int sockfd,
                           const InetAddress& local,
                           const InetAddress& peer)> NewConnectionCallback;

typedef std::function<void()> Task;
typedef std::function<void(size_t index)> ThreadInitCallback;
typedef std::function<void()> TimerCallback;

#endif //CALLBACKS_H
