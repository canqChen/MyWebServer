#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H

#include "SqlConnPool.h"

// 管理MySQL连接，资源在对象构造初始化，资源在对象析构时释放
class SqlConnRAII {
public:
    SqlConnRAII(MYSQL** sql, SqlConnPool *connpool) {
        assert(connpool);
        *sql = connpool->getConn();
        sql_ = *sql;
        connPool_ = connpool;
    }
    
    ~SqlConnRAII() {
        if(sql_) { 
            connPool_->freeConn(sql_); 
        }
    }
    
private:
    MYSQL *sql_;
    SqlConnPool* connPool_;
};

#endif //SQLCONNRAII_H