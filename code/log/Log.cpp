
#include "Log.h"

using namespace std;

Log::Log()
{
    mLineCount = 0;
    mWriteThread = nullptr;
    mQueue = nullptr;
    mToday = 0;
    mFp = nullptr;
    mLogIdx = 0;
}

Log::~Log()
{
    if (mWriteThread && mWriteThread->joinable())
    {
        while (!mQueue->Empty())
        {
            mQueue->Flush();
        }
        mQueue->Close();
        mWriteThread->join();
    }
    if (mFp)
    {
        lock_guard<mutex> locker(mMtx);
        Flush();
        fclose(mFp);
    }
}

LogLevel Log::GetLevel()
{
    // 运行过程更改日志级别需要加锁
    lock_guard<mutex> locker(mMtx);
    return mLevel;
}

void Log::SetLevel(int level)
{
    lock_guard<mutex> locker(mMtx);
    mLevel = level;
}

void Log::Init(LogLevel level = INFO, const char *path, const char *suffix,
               int maxQueueSize)
{
    assert(maxQueueSize > 0);
    mLevel = level;

    if (!mQueue)
    {
        unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>);
        mQueue = std::move(newQueue);

        std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread)); // 创建线程异步写日志
        mWriteThread = std::move(NewThread);
    }

    mLineCount = 0;
    mPath = path;
    mSuffix = suffix;

    auto sysTime = GetSysTime();
    // 短时间内重启服务器，mLogIdx可能不是从0开始，不过因为文件名加入了pid，所以重叠的可能性很小
    DetermineLogIdx(sysTime);
    auto fileName = GetFileName(sysTime);
    OpenFile(fileName);
}

string Log::GetFileName(struct tm sysTime){
    char fileName[LOG_NAME_LEN] = {'\0'};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d-%02d-%02d-%d-%d%s",
             mPath, sysTime.tm_year + 1900, sysTime.tm_mon + 1, sysTime.tm_mday, getpid(), mLogIdx, mSuffix); // 命名为 年-月-日-pid-序号.log 的日志文件
    mToday = sysTime.tm_mday;
    return string(fileName);
}

// 获取当前系统时间
struct tm Log::GetSysTime(){
    time_t now = time(nullptr);
    struct tm *sysTime = localtime(&now);
    return *sysTime;
}

void Log::OpenFile(string fileName){
    if(access(mPath, F_OK) != 0) {
        umask(0);
        mkdir(mPath, 0777);
    }
    if(mFp != nullptr) {
        // 清空缓冲区
        fflush(mFp);
        fclose(mFp);
        mFp == nullptr;
    }
    // 创建或打开文件
    mFp = fopen(fileName.c_str(), "a");
    assert(mFp != nullptr);
}

// 只在init时使用，确定起始文件名(mLogIdx)
void Log::DetermineLogIdx(struct tm sysTime) {
    while(access(GetFileName(sysTime), F_OK) == 0) {
        ++mLogIdx;
    }
}

void Log::LogBase(LogLevel level, const char *format, ...)
{
    if (IsOpen && int(GetLevel()) <= int(level))
    {
        va_lsit vaList;
        va_start(vaList, format);
        Write(level, format, vaList);
        Flush();
        va_end(vaList);
    }
}

// 检查写入日志日期是否为保存的mToday，日志行数是否超过最大限制，不满足则新建文件
void Log::UpdateWritenFile(struct tm sysTime){
    string fileName;
    // 将日志队列里面的旧日志清空
    Flush();
    // 如果是时间不是今天，则创建今天的日志，更新mToday和mLineCount
    if (mToday != t.tm_mday)
    {
        // 第二天的日志文件，从0开始计数
        mLogIdx = 0;
        // 新日志文件，行数从0开始
        mLineCount = 0;
        fileName = GetFileName(sysTime);
        // 更新“今天”
        mToday = sysTime.tm_mday;
    }
    else if(mLineCount >= MAX_LINES) // 单文件超过了最大行
    {
        // 新文件名序号递增
        ++mLogIdx;
        // 新日志文件，行数从0开始
        mLineCount = 0;
        fileName = GetFileName(sysTime);
    }
    OpenFile(fileName);
}

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

// 将日志按标准格式整理，写入阻塞队列中
void Log::Write(LogLevel level, const char *format, va_list vaList)
{
    // 加锁检查是否需要更新写入的日志文件
    // 获取当前系统时间
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);

    unique_lock<mutex> locker(mMtx);
    ++mLineCount;      // 行数 + 1

    // 如果是时间不是今天，或单文件超过了最大行
    if(mToday != t.tm_mday || mLineCount >= MAX_LINES){
        UpdateWritenFile(*sysTime);
    }
    
    locker.unlock();

    // 日志时间戳
    int n = snprintf(mBuff.writePosPointer(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",  // 年-月-日 hh-mm-ss.us
                        sysTime->tm_year + 1900, sysTime->tm_mon + 1, sysTime->tm_mday,
                        sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec, tv.tv_usec);

    mBuff.UpdateWritePos(n);
    AppendLogLevelTitle(level);

    int m = vsnprintf(mBuff.writePosPointer(), mBuff.WritableBytes(), format, vaList);

    mBuff.UpdateWritePos(m);
    mBuff.Append("\n\0", 2);

    // 将日志信息加入阻塞队列,同步则加锁向文件中写
    // 队列满，则阻塞等待，ClearAllToStr清空buff并返回string
    mQueue->PushBack(mBuff.RetriveToStr());
}

void Log::AppendLogLevelTitle(LogLevel level)
{
    switch (level)
    {
    case DEBUG:
        mBuff.Append("[debug]: ", 9);
        break;
    case INFO:
        mBuff.Append("[info] : ", 9);
        break;
    case WARN:
        mBuff.Append("[warn] : ", 9);
        break;
    case ERROR:
        mBuff.Append("[error]: ", 9);
        break;
    default:
        mBuff.Append("[info] : ", 9);
        break;
    }
}

// 插入一则日志，flush一次，唤醒一个消费者(Pop)，AsyncWrite 执行异步写
void Log::Flush()
{
    mQueue->Flush();
    fflush(mFp); // fflush()会强迫将缓冲区内的数据写回参数mFp指定的文件中，防止与下次写入日志混合
}

void Log::AsyncWrite()
{
    string str = "";
    while (mQueue->Pop(str))  // 队列空，会阻塞
    {
        lock_guard<mutex> locker(mMtx);
        fputs(str.c_str(), mFp);
    }
}

Log *Log::GetInstance()
{
    static Log inst;
    return &inst;
}

void Log::FlushLogThread()
{
    Log::GetInstance()->AsyncWrite();
}