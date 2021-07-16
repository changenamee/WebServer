#include <iostream>
#include "webserver.h"

int main() {
    WebServer server(
        //端口 trigger模式，timeout，linger
        1316, 3, 60000, false,        
        //mysql
        3306, "root", "aqaa", "mydb", 
        //connNum threadNum
        12, 6);            
    server.Start();
    return 0;
}
