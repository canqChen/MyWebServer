
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Epoller.h"
#include "../log/Log.h"
#include "../timer/TimerHeap.h"
#include "../pool/SqlConnPool.h"
#include "../pool/ThreadPool.h"
#include "../pool/SqlConnRAII.h"
#include "../http/HttpClient.h"

class WebServer {
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~WebServer();
    void Start();

private:
    bool InitSocket(); 
    void InitTrigerMode(int trigMode);
    void AddClient(int fd, sockaddr_in addr);
  
    void DealListen();
    void DealWrite(HttpClient* client);
    void DealRead(HttpClient* client);

    void SendError(int fd, const char*info);
    void ExtentTime(HttpClient* client);
    void CloseConn(HttpClient* client);

    void OnRead(HttpClient* client);
    void OnWrite(HttpClient* client);
    void OnProcess(HttpClient* client);

    static const int MAX_FD = 65536;   // 最大连接数

    static int SetFdNonblock(int fd);   // 将fd设置为非阻塞

    int mSocketPort;
    bool mOpenLinger;    // 是否优雅关闭
    int mTimeoutMS;  /* 超时时间毫秒MS，超过此值无响应，则关闭连接 */
    bool mClose;
    int mListenFd;   // 服务器socket监听fd
    char* mSrcDir;
    
    uint32_t _listenTrigerMode;    // 监听socket fd事件的触发模式
    uint32_t _connTrigerMode;        // 已连接的socket fd的事件的触发模式
   
    // RAII, Resource Acquisition Is Initialization  在构造函数中申请分配资源，在析构函数中释放资源

    std::unique_ptr<TimerHeap> mTimer;   
    std::unique_ptr<ThreadPool> mThreadpool;
    std::unique_ptr<Epoller> mEpoller;
    std::unordered_map<int, HttpClient> mClients;
};


#endif //WEBSERVER_H