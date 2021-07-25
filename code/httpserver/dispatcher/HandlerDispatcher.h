#ifndef HANDLERDISPATCHER_H
#define HANDLERDISPATCHER_H

#include<string>
#include<map>
#include<memory>

#include "../handler/AbstractHandler.h"
#include "../HttpCallbacks.h"


class HandlerDispatcher
{
public:
    static HandlerDispatcher* getInstance();

    void register_(const std::string & uri, const std::string &method, HandlerCallBack && handler);
    HandlerCallBack getHandler(const std::string & uri, const std::string &method);

    HandlerDispatcher(const HandlerDispatcher&) = delete;
    HandlerDispatcher(HandlerDispatcher&&) = delete;
    HandlerDispatcher & operator = (const HandlerDispatcher&) = delete;
    HandlerDispatcher & operator = (HandlerDispatcher&&) = delete;
    ~HandlerDispatcher();

private:
    HandlerDispatcher();
    std::map<std::string, pair<std::string, HandlerCallBack> > handlers_;
};

#endif