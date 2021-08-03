#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <functional>

#include "InetAddress.h"
#include "Channel.h"
#include "../common/NoCopyable.h"

class EventLoop;
class InetAddress;

class Connector: NoCopyable
{
public:
    Connector(EventLoop* loop, const InetAddress& peer);
    ~Connector();

    void start();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { 
        newConnectionCallback_ = cb; 
    }

    void setErrorCallback(const ErrorCallback& cb)
    { 
        errorCallback_ = cb; 
    }

private:
    void __handleWrite();

    EventLoop* loop_;
    const InetAddress peer_;
    const int sockfd_;
    bool connected_;
    bool started_;
    Channel channel_;
    NewConnectionCallback newConnectionCallback_;
    ErrorCallback errorCallback_;
};



#endif //CONNECTOR_H
