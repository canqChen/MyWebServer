#include "MainReactor.h"

using namespace std;

MainReactor::MainReactor(
            int port, int trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueSize):
            socketPort_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
            timer_(new TimerHeap()), threadPool_(new ThreadPool(threadNum)), epoller_(new Epoller())
    {
    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources", 16);
    HttpClient::userCount = 0;
    HttpClient::srcDir = srcDir_;       // 资源路径
    SqlConnPool::GetInstance()->init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);  // 初始化sql连接池

    __initTrigerMode(trigMode);
    if(!__initSocket()) { isClose_ = true;}

    if(openLog) {
        Log::getInstance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", socketPort_, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenTrigerMode_ & EPOLLET ? "ET": "LT"),
                            (connTrigerMode_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpClient::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

MainReactor::~MainReactor() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::GetInstance()->closePool();
}

void MainReactor::__initTrigerMode(int trigMode) {
    listenTrigerMode_ = EPOLLRDHUP;
    connTrigerMode_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connTrigerMode_ |= EPOLLET;
        break;
    case 2:
        listenTrigerMode_ |= EPOLLET;
        break;
    case 3:
        listenTrigerMode_ |= EPOLLET;
        connTrigerMode_ |= EPOLLET;
        break;
    default:
        listenTrigerMode_ |= EPOLLET;
        connTrigerMode_ |= EPOLLET;
        break;
    }
    HttpClient::isET = (connTrigerMode_ & EPOLLET);
}

void MainReactor::start() {
    int timeMS = -1;  // epoll wait timeout == -1 无事件将持续阻塞 
    if(!isClose_) { LOG_INFO("========== Server start =========="); }
    while(!isClose_) {
        if(timeoutMS_ > 0) {
            timeMS = timer_->getNextTick();         // 距离最短到时无响应任务的事件间隔
        }
        int eventCnt = epoller_->wait(timeMS);    // Reactor 事件处理模式
        for(int i = 0; i < eventCnt; i++) {
            // 处理事件
            int fd = epoller_->getFdByEvent(i);
            uint32_t events = epoller_->getEvent(i);
            if(fd == listenFd_) {    // 监听soket有新连接
                __dealListen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {      // 出现异常，关闭用户连接
                assert(clients_.count(fd) > 0);
                __closeConn(&clients_[fd]);
            }
            else if(events & EPOLLIN) {   // 可读
                assert(clients_.count(fd) > 0);
                __dealRead(&clients_[fd]);
            }
            else if(events & EPOLLOUT) {   // 可写
                assert(clients_.count(fd) > 0);
                __dealWrite(&clients_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void MainReactor::__sendError(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void MainReactor::__closeConn(HttpClient* client) {   // 关闭连接，并把对应fd监听事件从epoll监听队列中移除
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());
    epoller_->delFd(client->getFd());
    client->close();
}

void MainReactor::__addClient(int fd, struct sockaddr_in client_addr) {
    assert(fd > 0);
    clients_[fd].Init(fd, client_addr);   // 新增一个连接的客户端，并初始化其fd和address
    if(timeoutMS_ > 0) {
        timer_->addNode(fd, timeoutMS_, std::bind(&MainReactor::__closeConn, this, &clients_[fd]));   // 给新连接设置定时器，超时，则为非活动连接，应调用关闭连接回调函数
    }
    epoller_->addFd(fd, EPOLLIN | connTrigerMode_);  // 注册epoll事件，ET模式读入请求
    __setFdNonblock(fd);              // 设为非阻塞fd，使用非阻塞io
    LOG_INFO("Client[%d] in!", clients_[fd].getFd());
}

// 处理新连接
void MainReactor::__dealListen() {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr *)&client_addr, &len);   // mListenFd非阻塞，立刻返回，因为listenFd_非阻塞，有就绪连接才accept
        if(fd <= 0) { return;}          // accept监听队列已无established连接
        else if(HttpClient::userCount >= MAX_FD) {    // 超过最大连接数
            __sendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        __addClient(fd, client_addr);   // 添加客户
    } while(listenTrigerMode_ & EPOLLET);  // ET模式，只通知一次，每当有通知时，需要一次将所有连接读出，否则不再通知
}

// 处理可读事件
void MainReactor::__dealRead(HttpClient* client) {
    assert(client);
    __extentTime(client);   // 有响应，更新连接超时时间
    threadPool_->addTask(std::bind(&MainReactor::__onRead, this, client));  // 线程池处理读事件
}

void MainReactor::__onRead(HttpClient* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);  // 读数据到buffer
    if(ret <= 0 && readErrno != EAGAIN) {   // 出错关闭
        __closeConn(client);
        return;
    }
    __onProcess(client);        // 处理读出的数据
}

// 处理可写事件
void MainReactor::__dealWrite(HttpClient* client) {
    assert(client);
    __extentTime(client);
    threadPool_->addTask(std::bind(&MainReactor::__onWrite, this, client));
}

void MainReactor::__onWrite(HttpClient* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->send(&writeErrno);
    if(client->bytesToWrite() == 0) {       // 传输完成
        if(client->isKeepAlive()) {
            __onProcess(client);      // 长连接，监听可读事件
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {  // 继续传输
            epoller_->modFd(client->getFd(), connTrigerMode_ | EPOLLOUT);
            return;
        }
    }
    __closeConn(client);      // 短连接，关闭连接
}

// 更新socket连接(客户)的超时时间
void MainReactor::__extentTime(HttpClient* client) {
    assert(client);
    if(timeoutMS_ > 0) { timer_->update(client->getFd(), timeoutMS_); }
}

void MainReactor::__onProcess(HttpClient* client) {
    if(client->process()) {         // 处理完请求数据，然后监听可写事件
        epoller_->modFd(client->getFd(), connTrigerMode_ | EPOLLOUT);
    }
    else {              // 无数据处理，继续监听可读事件
        epoller_->modFd(client->getFd(), connTrigerMode_ | EPOLLIN);
    }
}

// Create listenFd
bool MainReactor::__initSocket() {
    int ret;
    struct sockaddr_in addr;
    if(socketPort_ > 65535 || socketPort_ < 1024) {
        LOG_ERROR("Port:%d error!",  socketPort_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);       // 本机所有网卡ip地址
    addr.sin_port = htons(socketPort_);

    struct linger optLinger = { 0 };
    if(openLinger_) {
        // 优雅关闭: 直到所剩数据发送完毕或超时
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error!", socketPort_);
        return false;
    }

    // 设置是否优雅关闭
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", socketPort_);
        return false;
    }

    int optval = 1;
    // 端口复用
    // time wait 状态的端口被强制使用
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", socketPort_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", socketPort_);
        close(listenFd_);
        return false;
    }
    ret = epoller_->addFd(listenFd_,  listenTrigerMode_ | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    __setFdNonblock(listenFd_);  // IO非阻塞，阻塞在epoll
    LOG_INFO("Server port:%d", socketPort_);
    return true;
}

// 设置非阻塞
int MainReactor::__setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}


