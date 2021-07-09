
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
#include "../blockingQueue/BlockQueue.h"
#include "../buffer/Buffer.h"

enum LogLevel{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Log {
public:
    void init(LogLevel level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    static Log* getInstance();
    LogLevel getLevel();
    void setLevel(LogLevel level);
    void logBase(LogLevel level, const char * format, ...);
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    Log& operator=(const Log&) = delete;
private:
    Log();
    virtual ~Log();
    
    void __asyncWrite();
    void __appendLogLevelTitle(LogLevel level, Buffer & buff);
    void __flush();
    void __flushAll();
    void __write(LogLevel level, const char *format, va_list vaList);
    static void __flushLogThread();
    bool __isFileOpen() const {return fp_ != nullptr;}

    string __genFileName(struct tm sysTime);
    void __changeLogFile(struct tm sysTime);
    void __determineLogIdx(struct tm sysTime);
    void __openFile(string fileName);
    struct tm Log::__getSysTime();

private:
    const uint32_t LOG_NAME_LEN = 256;
    const uint32_t MAX_LINES = 50000;

    const char* logPath_;
    const char* suffix_;

    size_t lineCount_;

    int today_;
    int fileIdx_;

    LogLevel level_;

    FILE* fp_;
    std::unique_ptr<BlockQueue<std::string> > blockQueue_;  // 封装阻塞队列
    std::unique_ptr<std::thread> writeThread_;  // 写日志线程
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::getInstance();\
        log->logBase(leve, format, ##__VA_ARGS__); \
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(DEBUG, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(INFO, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(WARN, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(ERROR, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H