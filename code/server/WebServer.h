
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
    void start();

private:
    bool __initSocket(); 
    void __initTrigerMode(int trigMode);
    void __addClient(int fd, sockaddr_in addr);
  
    void __dealListen();
    void __dealWrite(HttpClient* client);
    void __dealRead(HttpClient* client);

    void __sendError(int fd, const char*info);
    void __extentTime(HttpClient* client);
    void __closeConn(HttpClient* client);

    void __onRead(HttpClient* client);
    void __onWrite(HttpClient* client);
    void __onProcess(HttpClient* client);

    static const int MAX_FD = 65536;   // 最大连接数

    static int __setFdNonblock(int fd);   // 将fd设置为非阻塞

    int socketPort_;
    bool openLinger_;    // 是否优雅关闭
    int timeoutMS_;  /* 超时时间毫秒MS，超过此值无响应，则关闭连接 */
    bool isClose_;
    int listenFd_;   // 服务器socket监听fd
    char* srcDir_;
    
    uint32_t listenTrigerMode_;    // 监听socket fd事件的触发模式
    uint32_t connTrigerMode_;        // 已连接的socket fd的事件的触发模式
   
    // RAII, Resource Acquisition Is Initialization  在构造函数中申请分配资源，在析构函数中释放资源

    std::unique_ptr<TimerHeap> timer_;   
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpClient> clients_;
};


#endif //WEBSERVER_H