#include <unistd.h>
#include "httpserver/HttpServer.h"
#include "httpserver/Config.h"

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
    InetAddress local(Config::SERVERK_PORT);
    HttpServer server(local, Config::LOOPS, Config::WORKERS);
    server.start();

    return 0;
} 