
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

    const char * GetDBName() const;

    SqlConnPool() = delete;
    SqlConnPool(const SqlConnPool &) = delete;
    SqlConnPool & operator = (const SqlConnPool&) = delete;

private:
    ~SqlConnPool();

    std::string  dbName_;
    int maxConn_;
    int useCount_;
    int freeCount_;

    std::queue<MYSQL *> connQueue_;  // sql连接池队列
    std::mutex mtx_;        // 互斥锁
    sem_t semId_;       // 信号量，初始值为最大连接数
};


#endif // SQLCONNPOOL_H