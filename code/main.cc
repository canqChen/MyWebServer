#include <unistd.h>
#include "httpserver/HttpServer.h"
#include "httpserver/Config.h"
#include "common/Log.h"

int main() {
    // 守护进程 后台运行
    //daemon(1, 0); 
#ifndef NDEBUG
    Log::getInstance()->init(DEBUG);
#else
    Log::getInstance()->init(INFO);
#endif
    InetAddress local(Config::SERVERK_PORT);
    HttpServer server(local, Config::LOOPS, Config::WORKERS);
    server.start();

    return 0;
} 