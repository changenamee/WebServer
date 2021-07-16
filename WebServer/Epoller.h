//
// Created by sky on 2021/7/8.
//

#ifndef WEBSERVER_EPOLLER_H
#define WEBSERVER_EPOLLER_H
#include <sys/epoller.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <assert.h>
class Epoller {
public:
    explicit Epoller(int maxevents=100000);
    ~Epoller();
    bool AddFd(int fd,uint32_t events);
    bool ModFd(int fd,uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeOutMs=-1);
    uint32_t GetFdEvents(size_t i)  const;
    int GetFd(size_t i) const;

private:
    int epollFd;
    std::vector<struct epoll_event> events;
};


#endif //WEBSERVER_EPOLLER_H
