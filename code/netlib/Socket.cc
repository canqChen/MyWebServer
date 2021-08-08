#include "Socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common/Log.h"

int Socket::createSocket() {
    int ret = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (ret == -1) {
        // close(fd);
        LOG_FATAL("Create socket fail in %s", "Socket::createSocket() !");
    }
    return ret;
}

void Socket::setReuseAddr(const int fd)
{
    int on = 1;
    // timewait端口重用
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        ::close(fd);
        LOG_FATAL("Socket::setReuseAddr() set SO_REUSEADDR fail!");
    }
}

void Socket::setReusePort(const int fd)
{
    int on  = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    if (ret == -1) {
        ::close(fd);
        LOG_FATAL("Socket::setReusePort() set SO_REUSEPORT fail!");
    }
}

void Socket::setLinger(const int fd, int sec)
{
    struct linger optLinger = { 0 };
    optLinger.l_onoff = 1;
    optLinger.l_linger = sec; // 单位：s

    int ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        ::close(fd);
        LOG_FATAL("Socket::setLinger() set SO_LINGER fail!");
    }
}


int Socket::getSocketError(const int fd, int *err) 
{
    socklen_t len = sizeof(*err);
    return getsockopt(fd, SOL_SOCKET, SO_ERROR, err, &len);
}