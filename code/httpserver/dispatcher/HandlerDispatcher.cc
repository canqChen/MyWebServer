#include "./HandlerDispatcher.h"

HandlerDispatcher* HandlerDispatcher::getInstance() 
{
    static HandlerDispatcher instance;
    return &instance;
}

void HandlerDispatcher::register_(const std::string & uri, 
    const std::string &method, HandlerCallBack && handler)
{

}

HandlerCallBack HandlerDispatcher::getHandler(const std::string & uri, 
    const std::string &method)
{

}