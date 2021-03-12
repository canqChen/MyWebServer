#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>


class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;  
    size_t ReadableBytes() const ;  
    size_t HasReadBytes() const;

    const char* ReadBeginPointer() const;
    void EnsureWritable(size_t len);
    void UpdateWritten(size_t len);

    void UpdateRead(size_t len);
    void UpdateReadUntil(const char* end);

    void ClearAll() ;

    std::string ClearAllToStr();

    const char* NextWriteBeginPointerConst() const;
    char* NextWriteBeginPointer();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char*  BeginPtr();
    const char* BeginPtr() const;
    void MakeSpace(size_t len);

    std::vector<char> mBuffer;     // 缓存
    std::atomic<std::size_t> mReadPos;   // __buffer下次被读出的起始位置
    std::atomic<std::size_t> mWritePos;    // __buffer下次可被写入的起始位置
    // |----------------------------------------------|     mBuffer
    //        |mReadPos             |mWritePos
    //        |----> Readable <-----| --> Writable <--|
};

#endif //BUFFER_H