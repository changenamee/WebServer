#include <iostream>
#include "webserver.h"

int main() {
    WebServer server(
        //端口 trigger模式，timeout，linger
        1316, 3, 60000, false,        
        //mysql
        3306, "root", "aqaa", "mydb", 
        //连接池数量 线程池数量 日志开关 日志等级
        //connNum threadNum
        12, 6);            
    server.Start();
    return 0;
}
