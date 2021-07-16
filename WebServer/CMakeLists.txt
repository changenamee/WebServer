cmake_minimum_required(VERSION 3.19)
project(WebServer)

set(CMAKE_CXX_STANDARD 11)

add_executable(WebServer main.cpp Epoller.cpp Epoller.h WebServer.cpp WebServer.h Timer.cpp Timer.h ThreadPool.h HttpConn.cpp HttpConn.h SqlPool.cpp SqlPool.h SqlRAII.h httprequest.cpp httprequest.h httpresponse.cpp httpresponse.h buffer.cpp buffer.h)