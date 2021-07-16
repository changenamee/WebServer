//
// Created by sky on 2021/7/8.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H
#include <unordered_map>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "HttpConn.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "Epoller.h"
class WebServer {
public:
    WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,
              int sqlPort, const char* sqlUser, const  char* sqlPwd,
              const char* dbName, int connPoolNum, int threadNum,
              bool openLog, int logLevel, int logQueSize);
    ~WebServer();
    void start();
private:
    bool InitSocket();
    void InitEventMode(int eventMode);
    void SendErr(int fd,const char* err);
    void DealListen();
    void DealRead(HttpConn* client);
    void DealWrite(HttpConn* client);
    void OnRead(HttpConn* client);
    void OnWrite(HttpConn* client);
    void OnProcess(HttpConn* client);
    void ExtentTime(HttpConn* client);
    void CloseConn(HttpConn* client);
    void AddClient(int fd,sockaddr_in addr);
    static const int MAXFD=100000;
    static void SetNonBlock(int fd);


    uint32_t listenEvent;
    uint32_t connEvent;
    char *srcDir;
    bool isClosed;
    int timeOutMS;
    int listenfd;
    int port;
    bool openLinger;    //优雅关闭

    std::unique_ptr<ThreadPool> threadPool;
    std::unique_ptr<Timer> timer;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int,HttpConn> users;
};


#endif //WEBSERVER_WEBSERVER_H
