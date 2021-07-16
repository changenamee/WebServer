//
// Created by sky on 2021/7/17.
//

#ifndef WEBSERVER_SQLRAII_H
#define WEBSERVER_SQLRAII_H


#include "SqlPool.h"

/* 资源在对象构造初始化 资源在对象析构时释放*/
class SqlRAII {
public:
    SqlRAII(MYSQL** sql, SqlPool *connpool) {
        assert(connpool);
        *sql = connpool->GetConn();
        sql_ = *sql;
        connpool_ = connpool;
    }

    ~SqlRAII() {
        if(sql_) { connpool_->FreeConn(sql_); }
    }

private:
    MYSQL *sql_;    //mysql句柄
    SqlPool* connpool_;
};

#endif //WEBSERVER_SQLRAII_H
