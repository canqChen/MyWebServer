#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>   // perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> // readv
#include <vector>
#include <atomic>
#include <assert.h>
#include <algorithm>

using std::string;

class Buffer {
public:
    Buffer();
    Buffer(size_t initBuffSize, size_t headerSize);
    ~Buffer() = default;

    size_t writableBytes() const;  
    size_t readableBytes() const;
    size_t prependableBytes() const;

    const char* readPtr() const;
    char* readPtr();

    const char* writePtr() const;
    char* writePtr();

    void updateWritePos(size_t len);

    void updateReadPos(size_t len);
    void updateReadPos(const char* end);

    const char * findCRLF() const;
    const char * findDoubleCRLF() const;

    void clear();

    string retrieveAll();
    string retrieve(size_t len);
    string retrieveUtil(const char * pos);

    void append(const std::string& str);
    void append(const char* str, size_t len);
    void append(const void* data, size_t len);
    void append(const Buffer& buff);
    void append(string&& str);

    void prependInt64(int64_t x)
    {
        int64_t be64 = htobe64(x);
        __prepend(&be64, sizeof be64);
    }

    void prependInt32(int32_t x)
    {
        int32_t be32 = htobe32(x);
        __prepend(&be32, sizeof be32);
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = htobe16(x);
        __prepend(&be16, sizeof be16);
    }

    void prependInt8(int8_t x)
    { 
        __prepend(&x, sizeof x); 
    }

    ssize_t readFd(int fd, int* Errno);
    ssize_t writeFd(int fd, int* Errno);
private:
    char* __beginPtr();
    const char* __beginPtr() const;
    void __makeSpace(size_t len);
    void __ensureWritable(size_t len);

    void __prepend(const void *data, size_t len)
    {
        assert(len <= prependableBytes());
        readIdx_ -= len;
        auto d = static_cast<const char *>(data);
        std::copy(d, d + len, __beginPtr() + readIdx_);
    }

private:
    size_t headerSize_;
    std::vector<char> buff_;
    std::atomic<std::size_t> readIdx_;   // __buffer被读出的起始位置
    std::atomic<std::size_t> writeIdx_;    // __buffer可被写入的起始位置
    // |-------------------------------------------------------------|     buff_
    // |                     |readIdx_             |writeIdx_
    // |---> Prependable <---|----> Readable <-----| --> Writable <--|
};

#endif //BUFFER_H