#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H

#include "SqlConnPool.h"

// 管理MySQL连接，资源在对象构造初始化，资源在对象析构时释放
class SqlConnRAII {
public:
    SqlConnRAII(MYSQL** sql, SqlConnPool *connpool) {
        assert(connpool);
        *sql = connpool->GetConn();
        mSql = *sql;
        mConnPool = connpool;
    }
    
    ~SqlConnRAII() {
        if(mSql) { mConnPool->FreeConn(mSql); }
    }
    
private:
    MYSQL *mSql;
    SqlConnPool* mConnPool;
};

#endif //SQLCONNRAII_H