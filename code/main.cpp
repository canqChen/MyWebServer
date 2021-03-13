#include <unistd.h>
#include "server/WebServer.h"

int main() {
    // 守护进程 后台运行
    //daemon(1, 0); 

    WebServer server(
        1314, 3, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "root", "passwd", "userInfo", /* MySQL配置 端口 用户名 密码 数据库名称*/
        10, 8, true, 0, 1024);             /* MySQL连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    server.Start();
} 
  