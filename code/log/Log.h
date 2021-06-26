
#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include "BlockQueue.h"
#include "../buffer/Buffer.h"

enum LogLevel{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Log {
public:
    void Init(LogLevel level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    static Log* GetInstance();
    static void FlushLogThread();
    bool IsOpen() const {return mFp != nullptr;}
    LogLevel GetLevel();
    void SetLevel(LogLevel level);
    
    void LogBase(LogLevel level, const char * format, ...);
    
private:
    Log();
    virtual ~Log();
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    void AsyncWrite();
    void AppendLogLevelTitle(LogLevel level);
    void Flush();
    void Write(LogLevel level, const char *format, va_list vaList);
    
    string GetFileName(struct tm sysTime);
    void UpdateWritenFile(struct tm sysTime);
    void DetermineLogIdx(struct tm sysTime);
    void OpenFile(string fileName);
    struct tm Log::GetSysTime();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const uint32_t MAX_LINES = 50000;

    const char* mPath;
    const char* mSuffix;

    atomic_uint32_t mLineCount;

    int mToday;
    int mLogIdx;

    Buffer mBuff;
    LogLevel mLevel;

    FILE* mFp;
    std::unique_ptr<BlockQueue<std::string> > mQueue;  // 封装阻塞队列
    std::unique_ptr<std::thread> mWriteThread;  // 写日志线程
    std::mutex mMtx;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::GetInstance();\
        log->LogBase(leve, format, ##__VA_ARGS__); \
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(DEBUG, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(INFO, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(WARN, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(ERROR, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H