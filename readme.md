
## MyWebServer
* 利用 IO复用技术epoll(ET模式) + 非阻塞socket + 半同步/半反应堆线程池 + 单Reactor事件处理 实现的Web服务器；
* 利用正则表达式与有限状态机解析HTTP请求报文(GET和POST)，实现处理静态资源的请求；
* 利用标准库容器vector封装char，实现自动增长的缓冲区；
* 基于小根堆实现的定时器，关闭超时无响应的非活动连接；
* 利用RAII机制实现了数据库连接池，减少数据库连接建立与关闭的开销，同时实现了用户注册登录功能。



使用 线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现) 的并发模型
使用状态机解析HTTP请求报文，支持解析GET和POST请求
访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
实现同步/异步日志系统，记录服务器运行状态
经Webbench压力测试可以实现上万的并发连接数据交换


## 压力测试
![image-webbench](https://github.com/markparticle/WebServer/blob/master/readme.assest/%E5%8E%8B%E5%8A%9B%E6%B5%8B%E8%AF%95.png)
```bash
./webbench-1.5/webbench -c 100 -t 10 http://ip:port/
./webbench-1.5/webbench -c 1000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 5000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 10000 -t 10 http://ip:port/
```
* 测试环境: Ubuntu:20.10 cpu:i7-10750 内存:16G 
* QPS 10000+


## Reference
Linux高性能服务器编程，游双著.

[@mark](https://github.com/markparticle/WebServer)
