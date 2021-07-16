//
// Created by sky on 2021/7/16.
//

#include "SqlPool.h"
using namespace std;

SqlPool::SqlPool() {
    useCount_ = 0;
    freeCount_ = 0;
}

SqlPool* SqlPool::Instance() {
    static SqlPool connPool;
    return &connPool;
}

void SqlPool::Init(const char* host, int port,
                       const char* user,const char* pwd, const char* dbName,
                       int connSize = 10) {
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            assert(sql);
        }
        sql = mysql_real_connect(sql, host,
                                 user, pwd,
                                 dbName, port, nullptr, 0);
        connQue_.push(sql);
    }
    MAX_CONN_ = connSize;
    //最多可以有MAX_CONN_个数据库连接
    sem_init(&semId_, 0, MAX_CONN_);
}

MYSQL* SqlPool::GetConn() {
    MYSQL *sql = nullptr;
    if(connQue_.empty()){
        return nullptr;
    }
    sem_wait(&semId_);
    {
        lock_guard<mutex> locker(mtx_);
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}

void SqlPool::FreeConn(MYSQL* sql) {
    assert(sql);
    lock_guard<mutex> locker(mtx_);
    connQue_.push(sql);
    sem_post(&semId_);
}

void SqlPool::ClosePool() {
    lock_guard<mutex> locker(mtx_);
    while(!connQue_.empty()) {
        auto item = connQue_.front();
        connQue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int SqlPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx_);
    return connQue_.size();
}

SqlPool::~SqlPool() {
    ClosePool();
}
