
#include "Buffer.h"

Buffer::Buffer(int initBuffSize) : mBuffer(initBuffSize), mReadPos(0), mWritePos(0) {}

// __buffer中可被读出字节数
size_t Buffer::ReadableBytes() const {
    return mWritePos - mReadPos;
}

// __buffer中剩余可写入字节数
size_t Buffer::WritableBytes() const {
    return mBuffer.size() - mWritePos;
}

// 返回__buffer可被读起始位置
size_t Buffer::HasReadBytes() const {
    return mReadPos;
}

// 返回可读出起始位置的指针
const char* Buffer::ReadBeginPointer() const {
    return BeginPtr() + mReadPos;
}

// 更新下次可读起始位置 __readPos
void Buffer::UpdateRead(size_t len) {
    assert(len <= ReadableBytes());
    mReadPos += len;
}

// 更新已读光标至指定位置
void Buffer::UpdateReadUntil(const char* end) {
    assert(ReadBeginPointer() <= end );
    UpdateRead(end - ReadBeginPointer());
}

// 清空缓冲区
void Buffer::ClearAll() {
    bzero(&mBuffer[0], mBuffer.size());
    mReadPos = 0;
    mWritePos = 0;
}

// 保存__buffer数据并清空__buffer
std::string Buffer::ClearAllToStr() {
    std::string str(ReadBeginPointer(), ReadableBytes());
    ClearAll();
    return str;
}

// 返回下次可写入起始位置
const char* Buffer::NextWriteBeginPointerConst() const {
    return BeginPtr() + mWritePos;
}

// 返回下次可写入起始位置
char* Buffer::NextWriteBeginPointer() {
    return BeginPtr() + mWritePos;
}

// 更新下次写入起始位置的光标
void Buffer::UpdateWritten(size_t len) {
    mWritePos += len;
} 

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

// 写入__buffer，并更新下次写入起始位置
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, NextWriteBeginPointer());
    UpdateWritten(len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.ReadBeginPointer(), buff.ReadableBytes());
}

void Buffer::EnsureWritable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace(len);
    }
    assert(WritableBytes() >= len);
}

// 从socket fd读入__buffer
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];       // 申请大空间，防止mBuffer容量不足
    struct iovec iov[2];   // 分两部分读
    const size_t writable = WritableBytes();
    // 分散读，保证数据全部读完
    iov[0].iov_base = BeginPtr() + mWritePos;     // 起始地址
    iov[0].iov_len = writable;              // 长度
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);   // readv 按顺序填充iov
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {  // 小于等于writable，readv只读入__buffer中
        mWritePos += len;
    }
    else {                  // 超出__buffer
        mWritePos = mBuffer.size();   // __buffer满
        Append(buff, len - writable);   // 把超出部分拼接到__buffer后
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();  // __buffer中可被读出写至fd的字节数
    ssize_t len = write(fd, ReadBeginPointer(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    mReadPos += len;       // 已被读出len字节，更新下次可读出起始位置
    return len;
}

char* Buffer::BeginPtr() {
    return &*mBuffer.begin();
}

const char* Buffer::BeginPtr() const {
    return &*mBuffer.begin();
}

// 扩容或覆盖前面已读出数据
void Buffer::MakeSpace(size_t len) {
    if(WritableBytes() + HasReadBytes() < len) {  // 扩容len个子节
        mBuffer.resize(mWritePos + len + 1);
    }
    else {
        size_t readable = ReadableBytes();          // 覆盖__buffer前面已经读出的数据
        std::copy(BeginPtr() + mReadPos, BeginPtr() + mWritePos, BeginPtr());
        mReadPos = 0;
        mWritePos = mReadPos + readable;
        assert(readable == ReadableBytes());
    }
}