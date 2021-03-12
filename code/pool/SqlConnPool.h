
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/Log.h"

class SqlConnPool {
public:
    static SqlConnPool *GetInstance();

    MYSQL *GetConn();
    void FreeConn(MYSQL * conn);
    int GetFreeConnCount();

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);
    void ClosePool();

    SqlConnPool(const SqlConnPool &) = delete;
    SqlConnPool & operator = (const SqlConnPool&) = delete;

private:
    SqlConnPool();
    ~SqlConnPool();

    int mMaxConn;
    int mUseCount;
    int mFreeCount;

    std::queue<MYSQL *> mConnQueue;  // sql连接池队列
    std::mutex mMtx;        // 互斥锁
    sem_t mSemId;       // 信号量，初始值为最大连接数
};


#endif // SQLCONNPOOL_H