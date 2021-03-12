
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

class Log {
public:
    void Init(int level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    static Log* GetInstance();
    static void FlushLogThread();

    void Write(int level, const char *format,...);
    void Flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return mIsOpen; }
    
private:
    Log();
    void AppendLogLevelTitle(int level);
    virtual ~Log();
    void AsyncWrite();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* mPath;
    const char* mSuffix;

    int mLineCount;
    int mToday;

    bool mIsOpen;
 
    Buffer mBuff;
    int mLevel;
    bool mIsAsync;

    FILE* mFp;
    std::unique_ptr<BlockQueue<std::string> > mQueue;  // 封装阻塞队列
    std::unique_ptr<std::thread> mWriteThread;  // 写日志线程
    std::mutex mMtx;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::GetInstance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->Write(level, format, ##__VA_ARGS__); \
            log->Flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H