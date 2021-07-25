#include <unistd.h>
#include "./httpserver/HttpServer.h"



int logLevel;

#ifndef NDEBUG
logLevel = DEBUG;
#else
logLevel = INFO;
#endif

int main() {
    // 守护进程 后台运行
    //daemon(1, 0); 

    Log::getInstance()->init(logLevel);
    InetAddress local(1314);
    HttpServer server(local, 3, 3);
    server.start();

    return 0;
} 