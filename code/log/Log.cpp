
#include "Log.h"

using namespace std;

Log::Log()
{
    lineCount_ = 0;
    writeThread_ = nullptr;
    blockQueue_ = nullptr;
    today_ = 0;
    fp_ = nullptr;
    fileIdx_ = 0;
}

Log::~Log()
{
    if (writeThread_ && writeThread_->joinable())
    {
        while (!blockQueue_->empty())
        {
            blockQueue_->flush();
        }
        blockQueue_->close();
        writeThread_->join();
    }
    if (fp_)
    {
        lock_guard<mutex> locker(mtx_);
        __flush();
        fclose(fp_);
    }
}

LogLevel Log::getLevel()
{
    // 运行过程更改日志级别需要加锁
    lock_guard<mutex> locker(mtx_);
    return level_;
}

void Log::setLevel(int level)
{
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}

void Log::init(LogLevel level = INFO, const char *path, const char *suffix,
               int maxQueueSize)
{
    assert(maxQueueSize > 0);
    level_ = level;

    if (!blockQueue_)
    {
        unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>);
        blockQueue_ = std::move(newQueue);

        std::unique_ptr<std::thread> NewThread(new thread(__flushLogThread)); // 创建线程异步写日志
        writeThread_ = std::move(NewThread);
    }

    lineCount_ = 0;
    logPath_ = path;
    suffix_ = suffix;

    auto sysTime = __getSysTime();
    // 短时间内重启服务器，fileIdx_可能不是从0开始，不过因为文件名加入了pid，所以重叠的可能性很小
    __determineLogIdx(sysTime);
    auto fileName = __getFileName(sysTime);
    __openFile(fileName);
}

string Log::__getFileName(struct tm sysTime){
    char fileName[LOG_NAME_LEN] = {'\0'};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d-%02d-%02d-%d-%d%s",
             logPath_, sysTime.tm_year + 1900, sysTime.tm_mon + 1, sysTime.tm_mday, getpid(), fileIdx_, suffix_); // 命名为 年-月-日-pid-序号.log 的日志文件
    today_ = sysTime.tm_mday;
    return string(fileName);
}

// 获取当前系统时间
struct tm Log::__getSysTime(){
    time_t now = time(nullptr);
    struct tm *sysTime = localtime(&now);
    return *sysTime;
}

void Log::__openFile(string fileName){
    if(access(logPath_, F_OK) != 0) {
        umask(0);
        mkdir(logPath_, 0777);
    }
    if(fp_ != nullptr) {
        // 清空缓冲区
        fflush(fp_);
        fclose(fp_);
        fp_ == nullptr;
    }
    // 创建或打开文件
    fp_ = fopen(fileName.c_str(), "a");
    assert(fp_ != nullptr);
}

// 只在init时使用，确定起始文件名(fileIdx_)
void Log::__determineLogIdx(struct tm sysTime) {
    while(access(__getFileName(sysTime), F_OK) == 0) {
        ++fileIdx_;
    }
}

void Log::logBase(LogLevel level, const char *format, ...)
{
    if (__isFileOpen() && int(getLevel()) <= int(level))
    {
        va_lsit vaList;
        va_start(vaList, format);
        __write(level, format, vaList);
        __flush();
        va_end(vaList);
    }
}

// 检查写入日志日期是否为保存的today_，日志行数是否超过最大限制，不满足则新建文件
void Log::__changeWritenFile(struct tm sysTime){
    string fileName;
    // 将日志队列里面的旧日志清空
    __flush();
    // 如果是时间不是今天，则创建今天的日志，更新today_和lineCount_
    if (today_ != t.tm_mday)
    {
        // 第二天的日志文件，从0开始计数
        fileIdx_ = 0;
        // 新日志文件，行数从0开始
        lineCount_ = 0;
        fileName = __getFileName(sysTime);
        // 更新“今天”
        today_ = sysTime.tm_mday;
    }
    else if(lineCount_ >= MAX_LINES) // 单文件超过了最大行
    {
        // 新文件名序号递增
        ++fileIdx_;
        // 新日志文件，行数从0开始
        lineCount_ = 0;
        fileName = __getFileName(sysTime);
    }
    __openFile(fileName);
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
void Log::__write(LogLevel level, const char *format, va_list vaList)
{
    // 加锁检查是否需要更新写入的日志文件
    // 获取当前系统时间
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);

    unique_lock<mutex> locker(mtx_);
    ++lineCount_;      // 行数 + 1

    // 如果是时间不是今天，或单文件超过了最大行
    if(today_ != t.tm_mday || lineCount_ >= MAX_LINES){
        __changeWritenFile(*sysTime);
    }
    
    locker.unlock();

    // 日志时间戳
    int n = snprintf(buff_.writePosPointer(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",  // 年-月-日 hh-mm-ss.us
                        sysTime->tm_year + 1900, sysTime->tm_mon + 1, sysTime->tm_mday,
                        sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec, tv.tv_usec);

    buff_.updateWritePos(n);
    __appendLogLevelTitle(level);

    int m = vsnprintf(buff_.writePosPointer(), buff_.WritableBytes(), format, vaList);

    buff_.updateWritePos(m);
    buff_.append("\n\0", 2);

    // 将日志信息加入阻塞队列,同步则加锁向文件中写
    // 队列满，则阻塞等待，ClearAllToStr清空buff并返回string
    blockQueue_->push_back(buff_.RetriveToStr());
}

void Log::__appendLogLevelTitle(LogLevel level)
{
    switch (level)
    {
    case DEBUG:
        buff_.append("[debug]: ", 9);
        break;
    case INFO:
        buff_.append("[info] : ", 9);
        break;
    case WARN:
        buff_.append("[warn] : ", 9);
        break;
    case ERROR:
        buff_.append("[error]: ", 9);
        break;
    default:
        buff_.append("[info] : ", 9);
        break;
    }
}

// 插入一则日志，__flush一次，唤醒一个消费者(pop)，__asyncWrite 执行异步写
void Log::__flush()
{
    blockQueue_->flush();
    fflush(fp_); // fflush()会强迫将缓冲区内的数据写回参数fp_指定的文件中，防止与下次写入日志混合
}

void Log::__asyncWrite()
{
    string str = "";
    while (blockQueue_->pop(str))  // 队列空，会阻塞
    {
        lock_guard<mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

Log *Log::getInstance()
{
    static Log inst;
    return &inst;
}

void Log::__flushLogThread()
{
    Log::getInstance()->__asyncWrite();
}