
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

// http连接客户类，每个对象为一个客户
class HttpClient {
public:
    HttpClient();

    ~HttpClient();

    void Init(int sockFd, const sockaddr_in& addr);

    ssize_t Read(int* saveErrno);

    ssize_t Write(int* saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;
    
    sockaddr_in GetAddr() const;
    
    bool Process();

    // 待写字节数
    int ToWriteBytes() { 
        return mIov[0].iov_len + mIov[1].iov_len; 
    }

    bool IsKeepAlive() const {
        return mRequest.IsKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
    
private:
   
    int socketFd;       // socket连接的fd
    struct sockaddr_in mAddr;      // 客户地址， uint32_t

    bool mIsClose;
    
    int mIovCnt;
    struct iovec mIov[2];
    
    Buffer mReadBuff; // 读缓冲区
    Buffer mWriteBuff; // 写缓冲区

    HttpRequest mRequest;       // 处理请求
    HttpResponse mResponse;     // 处理响应
};


#endif //HTTP_CONN_H