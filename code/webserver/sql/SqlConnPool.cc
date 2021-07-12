

#include "SqlConnPool.h"
using namespace std;

SqlConnPool::SqlConnPool() {
    useCount_ = 0;
    freeCount_ = 0;
}

SqlConnPool* SqlConnPool::GetInstance() {
    static SqlConnPool connPool;
    return &connPool;
}

// sql连接池初始化
void SqlConnPool::init(const char* host, int port,
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
        connQueue_.push(sql);
    }
    dbName = std::move(std::string(dbName));
    maxConn_ = connSize;
    sem_init(&semId_, 0, maxConn_);
}

const char * SqlConnPool::getDBName() const{
    return dbName_.c_str();
}

// 获得一个连接
MYSQL* SqlConnPool::getConn() {
    MYSQL *sql = nullptr;
    if(connQueue_.empty()){
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&semId_);
    {
        lock_guard<mutex> locker(mtx_);   
        sql = connQueue_.front();
        connQueue_.pop();
    }
    return sql;
}

// 释放连接，归还sql连接池
void SqlConnPool::freeConn(MYSQL* sql) {
    assert(sql);
    lock_guard<mutex> locker(mtx_);
    connQueue_.push(sql);
    sem_post(&semId_);
}

void SqlConnPool::closePool() {
    lock_guard<mutex> locker(mtx_);
    while(!connQueue_.empty()) {
        auto item = connQueue_.front();
        connQueue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

// 空闲连接数量
int SqlConnPool::getFreeConnCount() {
    lock_guard<mutex> locker(mtx_);
    return connQueue_.size();
}

SqlConnPool::~SqlConnPool() {
    closePool();
}
