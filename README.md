# WebServer
c++实现的WebServer

应用技术：C++、TCP、epoll。                
项目描述：基于Linux的轻量级多线程Web服务器.
主要工作：
利用IO复用技术Epoll与线程池实现多线程的Reactor高并发模型；
利用状态机解析HTTP请求报文，实现处理静态资源的请求；
利用标准库容器封装char，实现自动增长的缓冲区；
基于小根堆实现的定时器，关闭超时的非活动连接；
利用RAII机制实现了数据库连接池，减少数据库连接建立与关闭的开销，同时实现了用户注册登录功能

## 环境要求
* Linux
* C++14
* MySql

#项目启动
make
./bin/server
