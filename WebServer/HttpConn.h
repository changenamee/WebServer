//
// Created by sky on 2021/7/8.
//

#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H
#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>
#include <unistd.h>
#include "SqlRAII.h"
#include "buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;

    sockaddr_in GetAddr() const;

    bool process();

    int ToWriteBytes() {
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;  //原子操作，c++11(atomic)

private:

    int fd_;
    struct  sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];   //io向量，读取或写入多个缓冲区

    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    HttpRequest request_;
    HttpResponse response_;
};


#endif //WEBSERVER_HTTPCONN_H
