
## MyWebServer
* 利用 IO复用技术epoll(ET模式)、非阻塞socket、半同步/半反应堆线程池、单Reactor事件处理模式， 实现的Web服务器；
* 利用正则表达式与有限状态机解析HTTP请求报文(GET和POST)，实现处理客户对静态资源的请求；
* 封装标准库容器vector构建缓冲区类，实现读写缓冲区自动增长；
* 基于小根堆实现的定时器，关闭超时无响应的非活动连接；
* 利用RAII机制实现了数据库连接池，减少数据库连接建立与关闭的开销，同时实现了用户注册登录功能。

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
