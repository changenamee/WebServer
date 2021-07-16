//
// Created by sky on 2021/7/16.
//

#ifndef WEBSERVER_SQLPOOL_H
#define WEBSERVER_SQLPOOL_H


#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
class SqlPool {
public:
    static SqlPool *Instance();

    MYSQL *GetConn();
    void FreeConn(MYSQL * conn);
    int GetFreeConnCount();

    void Init(const char* host, int port,
              const char* user,const char* pwd,
              const char* dbName, int connSize);
    void ClosePool();

private:
    SqlPool();
    ~SqlPool();

    int MAX_CONN_;
    int useCount_;
    int freeCount_;

    std::queue<MYSQL *> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};


#endif //WEBSERVER_SQLPOOL_H
