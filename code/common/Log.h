
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
#include "./BlockQueue.h"
#include "net/buffer/Buffer.h"
#include "./NoCopyable.h"

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Log : NoCopyable{
public:
    void init(LogLevel level, const char* path = "./log", 
                const char* suffix =".log");

    static Log* getInstance();
    // LogLevel getLevel();
    // void setLevel(LogLevel level);
    void logBase(const char * file, int line, LogLevel level, const char * format, ...);
private:
    Log();
    virtual ~Log();
    
    void __asyncWrite();
    void __appendLogLevelTitle(LogLevel level, Buffer & buff);
    void __flush();
    void __flushAll();
    void __write(const char * file, int line, LogLevel level, const char *format, va_list vaList);
    static void __flushLogThread();
    bool __isFileOpen() const {return fp_ != nullptr;}

    string __genFileName(struct tm sysTime);
    void __changeLogFile(struct tm sysTime);
    void __determineLogIdx(struct tm sysTime);
    void __openFile(string fileName);
    struct tm Log::__getSysTime();

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
    std::unique_ptr<BlockQueue<std::string> > blockQueue_;  // 封装阻塞队列
    std::unique_ptr<std::thread> writeThread_;  // 写日志线程
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...) \
        Log::getInstance()->logBase(__FILE__, __LINE__, leve, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) LOG_BASE(DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG_BASE(INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG_BASE(WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_BASE(ERROR, format, ##__VA_ARGS__)

#endif //LOG_H