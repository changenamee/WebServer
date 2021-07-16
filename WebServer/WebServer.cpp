//
// Created by sky on 2021/7/8.
//

#include "WebServer.h"


WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,
                     int sqlPort, const char* sqlUser, const  char* sqlPwd,
                     const char* dbName, int connPoolNum, int threadNum):port(port),timeOutMS(timeoutMS),
                     openLinger(OptLinger),isClosed(false),epoller_(new Epoller()),
                     threadPool(new ThreadPool(threadNum)),timer(new Timer())
{
    srcDir = getcwd(nullptr,255);
    assert(srcDir);
    strncat(srcDir,"/resources/",16);
    HttpConn::userCnt=0;
    HttpConn::srcDir=srcDir;

    InitEventMode(trigMode);
    if(!InitSocket())   isClosed = true;

    //log
}
WebServer::~WebServer() {
    close(listenfd);
    isClosed=true;
    free(srcDir);
    //关闭数据库连接
}
void WebServer::start() {
    int timeOut=-1;
    if(!isClosed){}   //log
    while(!isClosed){
        if(timeOutMS>0){
            timeOut = timer->GetNextTick();
        }
        int evenNum  = epoller_->Wait(timeOut);
        for(int i=0;i<evenNum;++i){
            int fd = epoller_->GetFd(i);
            uint32_t event = epoller_->GetFdEvents(i);
            if(fd==listenfd){
                DealListen();
            }else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                assert(users.count(fd)>0);
                CloseConn(&users[i]);
            }else if(event & EPOLLIN){
                assert(users.count(fd)>0);
                DealRead(&users[i]);
            }else if(event & EPOLLOUT){
                assert(users.count(fd)>0);
                DealWrite((&users[i]));
            }else{
                //log err conn
            }
        }
    }
}
void WebServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd>0);
    users[fd].Init(fd,addr);
    if(timeOutMS>0){}
    //设置超时时间
    timer->Add(fd,timeOutMS,std::bind(&WebServer::CloseConn,this,&users[fd]));
    epoller_->AddFd(fd,EPOLLIN|connEvent);
    SetNonBlock(fd);
    //log client add
}
void WebServer::CloseConn(HttpConn *client) {
    assert(client);
    //log client  quit
    epoller_->DelFd(client->GetFd());
    client->Close();
}
void WebServer::SendErr(int fd,const char* err){
    assert(fd>0);
    int ret = send(fd,err,strlen(err),0);
    if(ret<0){
        //log send fail;
    }
    close(fd);
}
void WebServer::DealListen() {
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    do{
        int connfd = accept(listenfd,(struct sockaddr*)&addr,&len);
        if(connfd<0)    return;
        else if(HttpConn::userCnt>=MAXFD){
            //log too many socket
            SendErr(connfd,"too many socket");
            close(connfd);
            return;
        }
        AddClient(connfd,addr);
    }while(listenEvent&EPOLLET);    //ET模式，循环处理
}
void WebServer::DealRead(HttpConn *client) {
    assert(client);
    ExtentTime(client);  //更新超时时间
    threadPool->AddTask((std::bind(&WebServer::OnRead,this,client)));
}
void WebServer::DealWrite(HttpConn *client) {
    assert(client);
    ExtentTime(client);
    threadPool->AddTask((std::bind(&WebServer::OnWrite,this,client)));
}
void WebServer::OnRead(HttpConn *client) {
    assert(client);
    int readErrno;
    int ret = client->Read(&readErrno);
    if(ret<0 && readErrno!=EAGAIN){
        CloseConn(client);
        return;
    }
    OnProcess(client);
}
void WebServer::OnProcess(HttpConn *client) {
    if(client->Process()){
        epoller_->ModFd(client->GetFd(), connEvent|EPOLLOUT);
    }else epoller_->ModFd(client->GetFd(), connEvent|EPOLLIN);
}
void WebServer::OnWrite(HttpConn *client) {
    assert(client);
    int writeErr;
    int ret = client->Write(&writeErr);
    if(client->BytesToWrite()==0){
        //传输完成
        //长连接，保持socket连接
        if(client->IsKeepAlive()){
            OnProcess(client);
            return;
        }
    }else if(ret<0){
        //有数据没有写完
        if(writeErr==EAGAIN){
            epoller_->ModFd(client->GetFd(), client->GetEvent()|EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}
void WebServer::InitEventMode(int eventMode) {
    listenEvent = EPOLLRDHUP;
    eventMode = EPOLLRDHUP|EPOLLONESHOT;
    switch (eventMode) {
        case 1:
            listenEvent |= EPOLLET;
            break;
        case 2:
            connEvent |= EPOLLET;
            break;
        case 3:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
            break;
        default:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
    }
    HttpConn::isET = (connEvent&EPOLLET);
}
bool WebServer::InitSocket() {
    int ret=0;
    if(port<1024 || port>65535)
            port = 9527;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    socklen_t len = sizeof(addr);
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0){
        //日志
        close(listenfd);
        return false;
    }
    //优雅关闭
    struct linger lingshut={0};
    if(openLinger){
        lingshut.l_onoff=1;     //关闭连接后如果有数据未发送，等待数据发送完毕
        lingshut.l_linger=1;    //延迟时间
    }
    ret = setsockopt(listenfd,SOL_SOCKET,SO_LINGER,(const void*)&lingshut,sizeof(lingshut));
    if(ret==-1){
        //log
        close(listenfd);
        return false;
    }
    //port复用
    int optval = 1;
    ret = setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(optval));
    if(ret==-1){
        //log
        close(listenfd);
        return false;
    }
    ret = bind(listenfd,(struct sockaddr*)&addr,len);
    if(ret==-1){
        //log
        close(listenfd);
        return false;
    }
    ret = listen(listenfd,6);
    if(ret==-1){
        //log
        close(listenfd);
        return false;
    }
    ret = epoller_->AddFd(listenfd,listenEvent|EPOLLIN);
    if(ret==0){
        //log
        close(listenfd);
        return false''
    }
    SetNonBlock(listenfd);
    //log port start
    return true;
}
void WebServer::SetNonBlock(int fd){
    fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|O_NONBLOCK);
}
