#include "Buffer.h"

Buffer::Buffer(size_t initBuffSize, size_t headerSize) 
    : headerSize_(headerSize), buff_(initBuffSize + headerSize), 
      readIdx_(headerSize_), writeIdx_(headerSize_) 
{
    assert(initBuffSize > 0 && headerSize_ > 0);
}

// __buffer中可被读出字节数
size_t Buffer::readableBytes() const 
{
    return writeIdx_ - readIdx_;
}

// buffer中剩余可写入字节数
size_t Buffer::writableBytes() const 
{
    return buff_.capacity() - writeIdx_;
}

// 返回buffer可被读起始位置
size_t Buffer::prependableBytes() const 
{
    return readIdx_;
}

// 返回可读出起始位置的指针
const char* Buffer::readPtr() const 
{
    return __beginPtr() + readIdx_;
}

char* Buffer::readPtr() 
{
    return __beginPtr() + readIdx_;
}

// 返回可写入起始位置
const char* Buffer::writePtr() const 
{
    return __beginPtr() + writeIdx_;
}

// 返回可写入起始位置
char* Buffer::writePtr() 
{
    return __beginPtr() + writeIdx_;
}

// 更新下次可读起始位置 readPos
void Buffer::updateReadPos(size_t len) 
{
    assert(len <= readableBytes());
    readIdx_ += len;
}

// 更新已读光标至指定位置
void Buffer::updateReadPos(const char* end) 
{
    assert(readPtr() <= end);
    updateReadPos(end - readPtr());
}

// 更新下次写入起始位置的光标
void Buffer::updateWritePos(size_t len) 
{
    writeIdx_ += len;
}

// 清空缓冲区
void Buffer::clear() 
{
    bzero(&buff_[0], buff_.capacity());
    readIdx_ = headerSize_;
    writeIdx_ = headerSize_;
}

// 保存buffer数据到string中
string Buffer::retrieveAll() 
{
    return retrieveUtil(readPtr() + readableBytes());
}

string Buffer::retrieve(size_t len) 
{
    return retrieveUtil(readPtr() + len);
}

string Buffer::retrieveUtil(const char * pos) 
{
    assert(pos >= readPtr());
    assert(pos <= writePtr());
    string ret = string(static_cast<const char*>(readPtr()), pos);
    return ret;
}

const char * Buffer::findCRLF() const 
{
    auto CRLF = "\r\n";
    const char* pos = std::search(readPtr(), writePtr(), CRLF, CRLF + 2);
    return pos == writePtr() ? nullptr : pos;
}

const char * Buffer::findDoubleCRLF() const 
{
    auto CRLF = "\r\n\r\n";
    const char* pos = std::search(readPtr(), writePtr(), CRLF, CRLF + 2);
    return pos == writePtr() ? nullptr : pos;
}

void Buffer::append(const string& str) 
{
    append(str.data(), str.length());
}

void Buffer::append(string&& str) 
{
    // string tmp(std::forward<string>(str));
    append(str.data(), str.length());
}

void Buffer::append(const void* data, size_t len) 
{
    assert(data!=nullptr);
    append(static_cast<const char*>(data), len);
}

// 写入buffer，并更新下次写入起始位置
void Buffer::append(const char* str, size_t len) 
{
    assert(str);
    if(len > 0) {
        __ensureWritable(len);
        std::copy(str, str + len, writePtr());
        updateWritePos(len);
    }
}

void Buffer::append(const Buffer& buff) {
    append(buff.readPtr(), buff.readableBytes());
}

// 从socket fd读入__buffer
ssize_t Buffer::readFd(int fd, int* saveErrno) 
{
    char buff[65535];       // 申请大空间，防止buff_容量不足
    struct iovec iov[2];   // 分两部分读
    const size_t writable = writableBytes();

    iov[0].iov_base = __beginPtr() + writeIdx_;     // 起始地址
    iov[0].iov_len = writable;              // 长度
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);   // readv 按顺序填充iov
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {  // 小于等于writable，readv只读入buffer中
        writeIdx_ += len;
    }
    else {                  // 超出buffer容量
        writeIdx_ = buff_.size();
        append(buff, len - writable);   // 把超出部分拼接到buffer后
    }
    return len;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno) 
{
    size_t readSize = readableBytes();  // buffer中可被读出写至fd的字节数
    ssize_t len = write(fd, readPtr(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    }
    updateReadPos(len);       // 已被读出len字节，更新下次可读出起始位置
    return len;
}

char* Buffer::__beginPtr() 
{
    return &*buff_.begin();
}

const char* Buffer::__beginPtr() const 
{
    return &*buff_.begin();
}

void Buffer::__ensureWritable(size_t len) 
{
    if(writableBytes() < len) {
        __makeSpace(len);
    }
    assert(writableBytes() >= len);
}

// 扩容或覆盖前面已读出数据
void Buffer::__makeSpace(size_t len) 
{
    size_t prependable = prependableBytes();
    size_t writable = writableBytes();
    
    // 扩容otherMore个子节
    if(len + headerSize_ > prependable + writable) {
        size_t otherMore = len + headerSize_ - prependable - writable;
        buff_.resize(buff_.capacity() + otherMore);
    }
    // 覆盖buffer前面已经读出的数据，数据前移
    size_t readable = readableBytes();
    std::copy(readPtr(), writePtr(), __beginPtr() + headerSize_);
    readIdx_ = headerSize_;
    writeIdx_ = readIdx_ + readable;
    assert(readable == readableBytes());
}