
#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>
#include <stdarg.h>           // vastart va_end
#include <cassert>

#include <queue>

#include "NoCopyable.h"

class Buffer;

enum LogLevel 
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    OFF
};

class Log : NoCopyable
{
public:
    void init(LogLevel level, const char* path = "./log", 
                const char* suffix =".log");
    void setLogLevel(LogLevel level);
    static Log* getInstance();
    LogLevel getLevel();
    void logBase(const char * file, int line, LogLevel level, bool to_abort, const char * format, ...);
private:
    Log();
    virtual ~Log();
    
    void __asyncWrite();
    void __appendLogLevelTitle(LogLevel level, Buffer & buff);
    void __write(const char * file, int line, LogLevel level, const char *format, va_list vaList);
    bool __isFileOpen() const {return fp_ != nullptr;}
    void __flush();
    std::string __genFileName(struct tm sysTime);
    void __changeLogFile(struct tm sysTime);
    void __determineLogIdx(struct tm sysTime);
    void __openFile(std::string fileName);
    struct tm __getSysTime();

private:
    const uint32_t LOG_NAME_LEN;
    const uint32_t MAX_LINES; // 一个日志文件10w行

    const char* logPath_;
    const char* suffix_;

    size_t lineCount_;

    int today_;
    int fileIdx_;

    LogLevel level_;

    FILE* fp_;
    std::queue<std::string> messageQueue_;  // 阻塞队列
    std::unique_ptr<std::thread> writeThread_;  // 写日志线程
    mutable std::mutex mtx_;
    std::condition_variable consumeCond_;
    volatile bool quit_;
};

#define LOG_BASE(level, to_abort, format, ...) (Log::getInstance()->logBase(__FILE__, __LINE__, level, to_abort, format, ##__VA_ARGS__))

#define LOG_TRACE(format, ...) if(Log::getInstance()->getLevel() <= TRACE) \ 
LOG_BASE(TRACE, false, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) if(Log::getInstance()->getLevel() <= DEBUG) \ 
LOG_BASE(DEBUG, false, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) if(Log::getInstance()->getLevel() <= INFO) \ 
LOG_BASE(INFO, false, format, ##__VA_ARGS__)

#define LOG_WARN(format, ...) LOG_BASE(WARN, false, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) LOG_BASE(ERROR, false, format, ##__VA_ARGS__)

#define LOG_FATAL(format, ...) LOG_BASE(FATAL, true, format, ##__VA_ARGS__)

#endif //LOG_H