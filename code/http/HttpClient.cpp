
#include "HttpClient.h"
using namespace std;

const char* HttpClient::srcDir;
std::atomic<int> HttpClient::userCount;
bool HttpClient::isET;

HttpClient::HttpClient() { 
    socketFd = -1;
    mAddr = { 0 };
    mIsClose = true;
};

HttpClient::~HttpClient() { 
    Close(); 
};

void HttpClient::Init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    mAddr = addr;
    socketFd = fd;
    mWriteBuff.ClearAll();
    mReadBuff.ClearAll();
    mIsClose = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", socketFd, GetIP(), GetPort(), (int)userCount);
}

void HttpClient::Close() {
    mResponse.UnmapFile();
    if(mIsClose == false){
        mIsClose = true; 
        userCount--;
        close(socketFd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", socketFd, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpClient::GetFd() const {
    return socketFd;
};

struct sockaddr_in HttpClient::GetAddr() const {
    return mAddr;
}

const char* HttpClient::GetIP() const {
    return inet_ntoa(mAddr.sin_addr);
}

int HttpClient::GetPort() const {
    return mAddr.sin_port;
}

ssize_t HttpClient::Read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = mReadBuff.ReadFd(socketFd, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);   // ET模式需循环读完
    return len;
}

ssize_t HttpClient::Write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(socketFd, mIov, mIovCnt);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        } 
        if(mIov[0].iov_len + mIov[1].iov_len  == 0) {   // 传输结束 
            break;
        } 
        else if(static_cast<size_t>(len) > mIov[0].iov_len) {       // 一次没写完，更新下次写的起始位置和待写的长度
            mIov[1].iov_base = (uint8_t*) mIov[1].iov_base + (len - mIov[0].iov_len);
            mIov[1].iov_len -= (len - mIov[0].iov_len);
            if(mIov[0].iov_len) {
                mWriteBuff.ClearAll();
                mIov[0].iov_len = 0;
            }
        }
        else {
            mIov[0].iov_base = (uint8_t*)mIov[0].iov_base + len; 
            mIov[0].iov_len -= len;
            mWriteBuff.UpdateRead(len);
        }
    } while(isET || ToWriteBytes() > 10240);
    return len;
}

bool HttpClient::Process() {
    mRequest.Init();
    if(mReadBuff.ReadableBytes() <= 0) {  // 无数据可处理
        return false;
    }
    else if(mRequest.Parse(mReadBuff)) {        // 解析请求http报文
        LOG_DEBUG("%s", mRequest.Path().c_str());
        mResponse.Init(srcDir, mRequest.Path(), mRequest.IsKeepAlive(), 200);       // 200 OK
    }
    else {
        mResponse.Init(srcDir, mRequest.Path(), false, 400);        // 400 请求错误
    }

    // 完成解析后准备响应内容

    mResponse.MakeResponse(mWriteBuff);
    // http响应状态行和消息头，在mIov[0]
    mIov[0].iov_base = const_cast<char*>(mWriteBuff.ReadBeginPointer());
    mIov[0].iov_len = mWriteBuff.ReadableBytes();
    mIovCnt = 1;

    // http消息体，由文件经mmap映射到内存中
    if(mResponse.FileLen() > 0  && mResponse.File()) {
        mIov[1].iov_base = mResponse.File();
        mIov[1].iov_len = mResponse.FileLen();
        mIovCnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", mResponse.FileLen() , mIovCnt, ToWriteBytes());
    return true;
}
