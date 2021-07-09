#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>   // perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
#include <algorithm>

using std::string;

class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t writableBytes() const;  
    size_t readableBytes() const;
    size_t prependableBytes() const;

    const char* readPtr() const;

    void updateWritePos(size_t len);

    void updateReadPos(size_t len);
    void updateReadPos(const char* end);

    const char * findCRLF() const;

    void clear();

    string retrieveAll2String();
    string retrieveUtil(const char * pos);

    const char* writePtr() const;
    char* writePtr();

    void append(const std::string& str);
    void append(const char* str, size_t len);
    void append(const void* data, size_t len);
    void append(const Buffer& buff);

    ssize_t readFd(int fd, int* Errno);
    ssize_t writeFd(int fd, int* Errno);
private:
    char* __beginPtr();
    const char* __beginPtr() const;
    void __makeSpace(size_t len);
    void __ensureWritable(size_t len);

private:
    std::vector<char> buff_;     // 缓存
    std::atomic<std::size_t> readIdx_;   // __buffer下次被读出的起始位置
    std::atomic<std::size_t> writeIdx_;    // __buffer下次可被写入的起始位置
    // |-------------------------------------------------------------|     buff_
    // |                     |readIdx_             |writeIdx_
    // |---> Prependable <---|----> Readable <-----| --> Writable <--|
};

#endif //BUFFER_H