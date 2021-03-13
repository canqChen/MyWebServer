#include "WebServer.h"

using namespace std;

WebServer::WebServer(
            int port, int trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueSize):
            mSocketPort(port), mOpenLinger(OptLinger), mTimeoutMS(timeoutMS), mClose(false),
            mTimer(new TimerHeap()), mThreadpool(new ThreadPool(threadNum)), mEpoller(new Epoller())
    {
    mSrcDir = getcwd(nullptr, 256);
    assert(mSrcDir);
    strncat(mSrcDir, "/resources", 16);
    HttpClient::userCount = 0;
    HttpClient::srcDir = mSrcDir;       // 资源路径
    SqlConnPool::GetInstance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);  // 初始化sql连接池

    InitTrigerMode(trigMode);
    if(!InitSocket()) { mClose = true;}

    if(openLog) {
        Log::GetInstance()->Init(logLevel, "./log", ".log", logQueSize);
        if(mClose) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", mSocketPort, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (mListenTrigerMode & EPOLLET ? "ET": "LT"),
                            (mConnTrigerMode & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpClient::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer() {
    close(mListenFd);
    mClose = true;
    free(mSrcDir);
    SqlConnPool::GetInstance()->ClosePool();
}

void WebServer::InitTrigerMode(int trigMode) {
    mListenTrigerMode = EPOLLRDHUP;
    mConnTrigerMode = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        mConnTrigerMode |= EPOLLET;
        break;
    case 2:
        mListenTrigerMode |= EPOLLET;
        break;
    case 3:
        mListenTrigerMode |= EPOLLET;
        mConnTrigerMode |= EPOLLET;
        break;
    default:
        mListenTrigerMode |= EPOLLET;
        mConnTrigerMode |= EPOLLET;
        break;
    }
    HttpClient::isET = (mConnTrigerMode & EPOLLET);
}

void WebServer::Start() {
    int timeMS = -1;  // epoll wait timeout == -1 无事件将持续阻塞 
    if(!mClose) { LOG_INFO("========== Server start =========="); }
    while(!mClose) {
        if(mTimeoutMS > 0) {
            timeMS = mTimer->GetNextTick();         // 距离最短到时无响应任务的事件间隔
        }
        int eventCnt = mEpoller->Wait(timeMS);    // Reactor 事件处理模式
        for(int i = 0; i < eventCnt; i++) {
            // 处理事件
            int fd = mEpoller->GetFdByEvent(i);
            uint32_t events = mEpoller->GetEvent(i);
            if(fd == mListenFd) {    // 监听soket有新连接
                DealListen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {      // 出现异常，关闭用户连接
                assert(mClients.count(fd) > 0);
                CloseConn(&mClients[fd]);
            }
            else if(events & EPOLLIN) {   // 可读
                assert(mClients.count(fd) > 0);
                DealRead(&mClients[fd]);
            }
            else if(events & EPOLLOUT) {   // 可写
                assert(mClients.count(fd) > 0);
                DealWrite(&mClients[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::SendError(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::CloseConn(HttpClient* client) {   // 关闭连接，并把对应fd监听事件从epoll监听队列中移除
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    mEpoller->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient(int fd, struct sockaddr_in client_addr) {
    assert(fd > 0);
    mClients[fd].Init(fd, client_addr);   // 新增一个连接的客户端，并初始化其fd和address
    if(mTimeoutMS > 0) {
        mTimer->AddNode(fd, mTimeoutMS, std::bind(&WebServer::CloseConn, this, &mClients[fd]));   // 给新连接设置定时器，超时，则为非活动连接，应调用关闭连接回调函数
    }
    mEpoller->AddFd(fd, EPOLLIN | mConnTrigerMode);  // 注册epoll事件，ET模式读入请求
    SetFdNonblock(fd);              // 设为非阻塞fd，使用非阻塞io
    LOG_INFO("Client[%d] in!", mClients[fd].GetFd());
}

// 处理新连接
void WebServer::DealListen() {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    do {
        int fd = accept(mListenFd, (struct sockaddr *)&client_addr, &len);   // 非阻塞，立刻返回，因为listenFd_非阻塞，有就绪连接才accept
        if(fd <= 0) { return;}          // accept监听队列已无established连接
        else if(HttpClient::userCount >= MAX_FD) {    // 超过最大连接数
            SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient(fd, client_addr);   // 添加客户
    } while(mListenTrigerMode & EPOLLET);  // ET模式，只通知一次，每当有通知时，需要一次将所有连接读出，否则不再通知
}

// 处理可读事件
void WebServer::DealRead(HttpClient* client) {
    assert(client);
    ExtentTime(client);   // 有响应，更新连接超时时间
    mThreadpool->AddTask(std::bind(&WebServer::OnRead, this, client));  // 线程池处理读事件
}

// 处理可写事件
void WebServer::DealWrite(HttpClient* client) {
    assert(client);
    ExtentTime(client);
    mThreadpool->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

// 更新socket连接(客户)的超时时间
void WebServer::ExtentTime(HttpClient* client) {
    assert(client);
    if(mTimeoutMS > 0) { mTimer->Update(client->GetFd(), mTimeoutMS); }
}

void WebServer::OnRead(HttpClient* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->Read(&readErrno);  // 读数据到__buffer
    if(ret <= 0 && readErrno != EAGAIN) {   // 出错关闭
        CloseConn(client);
        return;
    }
    OnProcess(client);        // 处理读出的数据
}

void WebServer::OnProcess(HttpClient* client) {
    if(client->Process()) {         // 处理完请求数据，然后监听可写事件
        mEpoller->ModFd(client->GetFd(), mConnTrigerMode | EPOLLOUT);
    } 
    else {              // 无数据处理，继续监听可读事件
        mEpoller->ModFd(client->GetFd(), mConnTrigerMode | EPOLLIN);
    }
}

void WebServer::OnWrite(HttpClient* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->Write(&writeErrno);
    if(client->ToWriteBytes() == 0) {       // 传输完成
        if(client->IsKeepAlive()) {
            OnProcess(client);      // 长连接，监听可读事件
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {  // 继续传输
            mEpoller->ModFd(client->GetFd(), mConnTrigerMode | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);      // 短连接，关闭连接
}

// Create listenFd
bool WebServer::InitSocket() {
    int ret;
    struct sockaddr_in addr;
    if(mSocketPort > 65535 || mSocketPort < 1024) {
        LOG_ERROR("Port:%d error!",  mSocketPort);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);       // 本机所有网卡ip地址
    addr.sin_port = htons(mSocketPort);

    struct linger optLinger = { 0 };
    if(mOpenLinger) {
        // 优雅关闭: 直到所剩数据发送完毕或超时
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    mListenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(mListenFd < 0) {
        LOG_ERROR("Create socket error!", mSocketPort);
        return false;
    }

    // 设置是否优雅关闭
    ret = setsockopt(mListenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(mListenFd);
        LOG_ERROR("Init linger error!", mSocketPort);
        return false;
    }

    int optval = 1;
    // 端口复用
    // 只有最后一个套接字会正常接收数据
    // time wait 状态的连接被强制使用
    ret = setsockopt(mListenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(mListenFd);
        return false;
    }

    ret = bind(mListenFd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", mSocketPort);
        close(mListenFd);
        return false;
    }

    ret = listen(mListenFd, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", mSocketPort);
        close(mListenFd);
        return false;
    }
    ret = mEpoller->AddFd(mListenFd,  mListenTrigerMode | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(mListenFd);
        return false;
    }
    SetFdNonblock(mListenFd);  // IO非阻塞，阻塞在epoll
    LOG_INFO("Server port:%d", mSocketPort);
    return true;
}

// 设置非阻塞
int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}


