#ifndef SOCKET_H
#define SOCKET_H

class Socket 
{
public:
    static int createSocket();
    static void setReuseAddr(const int fd);
    static void setReusePort(const int fd);
    static void setLinger(const int fd, int sec);
    static int getSocketError(const int fd, int * err);
};

#endif