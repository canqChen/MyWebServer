
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "../log/Log.h"
#include "../pool/SqlConnRAII.h"
#include "../buffer/Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

// http连接客户类，每个对象为一个http(长)连接
class HttpClient {
public:
    HttpClient();

    ~HttpClient();

    void init(int sockFd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t send(int* saveErrno);

    void close();

    int getFd() const;

    int getRemotePort() const;

    const char* getRemoteAddr() const;
    
    bool process();

    // 待写iov_中总字节数
    int ToWriteBytes() const {
        return iov_[0].iov_len + iov_[1].iov_len; 
    }

    bool isKeepAlive() const {
        return mRequest.isKeepAlive();
    }

    bool isET;
    static const char* srcDir;
    static std::atomic<size_t> userCount;
private:
    int socketFd_;       // socket连接的fd
    struct sockaddr_in addr_;      // 客户地址， uint32_t

    bool isClose_;
    
    int iovCnt_;
    struct iovec iov_[2];
    
    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    HttpRequest mRequest;       // 处理请求
    HttpResponse mResponse;     // 处理响应

    unique_ptr<HttpRequest> httpRequest_;
    unique_ptr<HttpResponse> httpResponse_;
};


#endif //HTTP_CONN_H