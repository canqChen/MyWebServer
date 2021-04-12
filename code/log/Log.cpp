
#include "Log.h"

using namespace std;

Log::Log() {
    mLineCount = 0;
    mIsAsync = false;
    mWriteThread = nullptr;
    mQueue = nullptr;
    mToday = 0;
    mFp = nullptr;
}

Log::~Log() {
    if(mWriteThread && mWriteThread->joinable()) {
        while(!mQueue->Empty()) {
            mQueue->Flush();
        }
        mQueue->Close();
        mWriteThread->join();
    }
    if(mFp) {
        lock_guard<mutex> locker(mMtx);
        Flush();
        fclose(mFp);
    }
}

int Log::GetLevel() {
    lock_guard<mutex> locker(mMtx);
    return mLevel;
}

void Log::SetLevel(int level) {
    lock_guard<mutex> locker(mMtx);
    mLevel = level;
}

void Log::Init(int level = 1, const char* path, const char* suffix,
    int maxQueueSize) {
    mIsOpen = true;
    mLevel = level;
    if(maxQueueSize > 0) {
        mIsAsync = true;
        if(!mQueue) {
            unique_ptr<BlockQueue<std::string> > newQueue(new BlockQueue<std::string>);
            mQueue = std::move(newQueue);
            
            std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread));   // 创建线程异步写日志
            mWriteThread = std::move(NewThread);
        }
    } 
    else {
        mIsAsync = false;
    }

    mLineCount = 0;

#if 0
struct tm {
        int tm_sec;         // seconds : The  number of seconds after the minute, normally in the range 0 to 59, but can be up to 60 to allow for leap seconds.
        int tm_min;         // minutes : The number of minutes after the hour, in the range 0 to 59.
        int tm_hour;        // hours : The number of hours past midnight, in the range 0 to 23.
        int tm_mday;        // day of the month : The day of the month, in the range 1 to 31.
        int tm_mon;         // month : The number of months since January, in the range 0 to 11.
        int tm_year;        // year : The number of years since 1900.
        int tm_wday;        // day of the week : The number of days since Sunday, in the range 0 to 6.
        int tm_yday;        // day in the year : The number of days since January 1, in the range 0 to 365.
        int tm_isdst;       // daylight saving time : A flag that indicates whether daylight saving time is in effect at the  time  described. The value is positive if daylight saving time is in effect, zero if it is not, and negative if the information is not available.
 
};
#endif

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    mPath = path;
    mSuffix = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            mPath, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, mSuffix);  // 命名为 年_月_日.log 的日志文件
    mToday = t.tm_mday;

    {
        lock_guard<mutex> locker(mMtx);
        mBuff.ClearAll();
        if(mFp) { 
            Flush();
            fclose(mFp); 
        }

        // 创建或打开文件

        mFp = fopen(fileName, "a"); 
        if(mFp == nullptr) {
            mkdir(mPath, 0777);
            mFp = fopen(fileName, "a");
        } 
        assert(mFp != nullptr);
    }
}

// 将日志按标准格式整理，写入阻塞队列中
void Log::Write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    // 日志日期是否为今天 日志行数是否超过最大限制
    if (mToday != t.tm_mday || (mLineCount && (mLineCount  %  MAX_LINES == 0)))
    {
        unique_lock<mutex> locker(mMtx);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        // 如果是时间不是今天,则创建今天的日志，更新mToday和mLineCount
        if (mToday != t.tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", mPath, tail, mSuffix);
            mToday = t.tm_mday;
            mLineCount = 0;
        }
        else {
            // 超过了最大行，在之前的日志名基础上加后缀, (mLineCount  / MAX_LINES)
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", mPath, tail, (mLineCount  / MAX_LINES), mSuffix);
        }
        
        locker.lock();
        Flush();
        fclose(mFp);
        mFp = fopen(newFile, "a");
        assert(mFp != nullptr);
    }

    {
        unique_lock<mutex> locker(mMtx);        // 格式化日志内容需要互斥(共用mBuff)
        mLineCount++;   // 行数+1
        int n = snprintf(mBuff.NextWriteBeginPointer(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        
        mBuff.UpdateWritten(n);
        AppendLogLevelTitle(level);

        // 将传入的format参数赋值给valst，便于格式化输出
        va_start(vaList, format);
        int m = vsnprintf(mBuff.NextWriteBeginPointer(), mBuff.WritableBytes(), format, vaList);
        va_end(vaList);

        mBuff.UpdateWritten(m);
        mBuff.Append("\n\0", 2);

        // 若mIsAsync为true表示异步，默认为同步
        // 若异步,则将日志信息加入阻塞队列,同步则加锁向文件中写
        if(mIsAsync && mQueue && !mQueue->Full()) {
            mQueue->Push_back(mBuff.ClearAllToStr());
        } else {
            fputs(mBuff.ReadBeginPointer(), mFp);   // 同步，直接写入
        }
        mBuff.ClearAll();
    }
}

void Log::AppendLogLevelTitle(int level) {
    switch(level) {
    case 0:
        mBuff.Append("[debug]: ", 9);
        break;
    case 1:
        mBuff.Append("[info] : ", 9);
        break;
    case 2:
        mBuff.Append("[warn] : ", 9);
        break;
    case 3:
        mBuff.Append("[error]: ", 9);
        break;
    default:
        mBuff.Append("[info] : ", 9);
        break;
    }
}

// 插入一则日志，flush一次，唤醒一个消费者(Pop)，AsyncWrite 执行异步写
void Log::Flush() {
    if(mIsAsync) { 
        mQueue->Flush(); 
    }
    fflush(mFp);   // fflush()会强迫将缓冲区内的数据写回参数mFp指定的文件中，防止与下次写入日志混合
}

void Log::AsyncWrite() {
    string str = "";
    while(mQueue->Pop(str)) {
        lock_guard<mutex> locker(mMtx);
        fputs(str.c_str(), mFp);
    }
}

Log* Log::GetInstance() {
    static Log inst;
    return &inst;
}

void Log::FlushLogThread() {
    Log::GetInstance()->AsyncWrite();
}