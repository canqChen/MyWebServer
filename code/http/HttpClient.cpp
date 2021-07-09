
#include "HttpClient.h"
using namespace std;

const char* HttpClient::srcDir;
std::atomic<size_t> HttpClient::userCount = 0;
bool HttpClient::isET;

HttpClient::HttpClient() { 
    socketFd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};

HttpClient::~HttpClient() {
    close(); 
};

void HttpClient::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    socketFd_ = fd;
    writeBuff_.clear();
    readBuff_.clear();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", socketFd_, getRemoteAddr(), getRemotePort(), (int)userCount);
}

void HttpClient::close() {
    mResponse.unmapFile();
    if(isClose_ == false) {
        isClose_ = true;
        userCount--;
        close(socketFd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", socketFd_, getRemoteAddr(), getRemotePort(), (int)userCount);
    }
}

int HttpClient::getFd() const {
    return socketFd_;
};

const char* HttpClient::getRemoteAddr() const {
    return inet_ntoa(addr_.sin_addr);
}

int HttpClient::getRemotePort() const {
    return addr_.sin_port;
}

ssize_t HttpClient::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.readFd(socketFd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);   // ET模式需循环读完
    return len;
}

ssize_t HttpClient::send(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(socketFd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        } 
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) {   // 传输结束 
            break;
        } 
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {       // 一次没写完，更新下次写的起始位置和待写的长度
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.clear();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len;
            writeBuff_.updateReadPos(len);
        }
    } while(isET || bytesToWrite() > 10240);
    return len;
}

bool HttpClient::process() {
    mRequest.__init();
    if(readBuff_.readableBytes() <= 0) {  // 无数据可处理
        return false;
    }
    else if(mRequest.parseRequest(readBuff_)) {        // 解析请求http报文
        LOG_DEBUG("%s", mRequest.Path().c_str());
        mResponse.init(srcDir, mRequest.Path(), mRequest.isKeepAlive(), 200);       // 200 OK
    }
    else {
        mResponse.init(srcDir, mRequest.Path(), false, 400);        // 400 请求错误
    }

    // 完成解析后准备响应内容
    mResponse.makeResponse(writeBuff_);
    // http响应状态行和消息头，在mIov[0]
    iov_[0].iov_base = const_cast<char*>(writeBuff_.readPtr());
    iov_[0].iov_len = writeBuff_.readableBytes();
    iovCnt_ = 1;

    // http消息体，由文件经mmap映射到内存中
    if(mResponse.fileSize() > 0  && mResponse.getFile()) {
        iov_[1].iov_base = mResponse.getFile();
        iov_[1].iov_len = mResponse.fileSize();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", mResponse.fileSize() , iovCnt_, bytesToWrite());
    return true;
}
