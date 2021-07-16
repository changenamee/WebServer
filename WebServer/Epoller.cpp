//
// Created by sky on 2021/7/8.
//

#include "Epoller.h"

Epoller::Epoller(int maxevents):epollFd(epoll_create(512)),events(maxevents){
    assert(epollFd>0 && events.size()>0);
}

Epoller::~Epoller() {
    close(epollFd);
}
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd<0)    return false;
    struct epoll_event et={0};
    et.data.fd=fd;
    et.events=events;
    return 0 == epoll_ctl(epollFd,EPOLL_CTL_ADD,fd,&et);
}
bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd<0)    return false;
    struct epoll_event et={0};
    et.data.fd=fd;
    et.events=events;
    return 0==epoll_ctl(epollFd,EPOLL_CTL_MOD,fd,&et);
}
bool Epoller::DelFd(int fd) {
    if(fd<0)    return false;
    struct epoll_event et={0};
    return 0==epoll_ctl(epollFd,EPOLL_CTL_DEL,fd,&et);
}

uint32_t Epoller::GetFdEvents(size_t i) const {
    assert(i<events.size() && i>=0);
    return events[i].data.events;
}
int Epoller::GetFd(size_t i) const {
    assert(i<events.size() && i>=0);
    return events[i].data.fd;
}