

#include "SqlConnPool.h"
using namespace std;

SqlConnPool::SqlConnPool() {
    mUseCount = 0;
    mFreeCount = 0;
}

SqlConnPool* SqlConnPool::GetInstance() {
    static SqlConnPool connPool;
    return &connPool;
}

// sql连接池初始化
void SqlConnPool::Init(const char* host, int port,
            const char* user,const char* pwd, const char* dbName,
            int connSize = 10) {
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host,
                                 user, pwd,
                                 dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySql Connect error!");
        }
        mConnQueue.push(sql);
    }
    mDBName = std::move(std::string(dbName));
    mMaxConn = connSize;
    sem_init(&mSemId, 0, mMaxConn);
}

const char * SqlConnPool::GetDBName() const{
    return mDBName.c_str();
}

// 获得一个连接
MYSQL* SqlConnPool::GetConn() {
    MYSQL *sql = nullptr;
    if(mConnQueue.empty()){
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&mSemId);
    {
        lock_guard<mutex> locker(mMtx);   
        sql = mConnQueue.front();
        mConnQueue.pop();
    }
    return sql;
}

// 释放连接，归还sql连接池
void SqlConnPool::FreeConn(MYSQL* sql) {
    assert(sql);
    lock_guard<mutex> locker(mMtx);
    mConnQueue.push(sql);
    sem_post(&mSemId);
}

void SqlConnPool::ClosePool() {
    lock_guard<mutex> locker(mMtx);
    while(!mConnQueue.empty()) {
        auto item = mConnQueue.front();
        mConnQueue.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

// 空闲连接数量
int SqlConnPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mMtx);
    return mConnQueue.size();
}

SqlConnPool::~SqlConnPool() {
    ClosePool();
}
