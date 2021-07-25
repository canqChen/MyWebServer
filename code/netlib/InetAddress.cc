//
// Created by frank on 17-9-1.
//

#include <arpa/inet.h>
#include <strings.h>

#include "../common/Log.h"
#include "./InetAddress.h"


InetAddress::InetAddress(uint16_t port, bool loopback) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    // INADDR_ANY是ANY，是绑定地址0.0.0.0上的监听, 能收到任意一块网卡的连接
    // INADDR_LOOPBACK, 也就是绑定地址LOOPBAC, 往往是127.0.0.1, 只能收到127.0.0.1上面的连接请求
    in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = htonl(ip);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    int ret = ::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr.s_addr);
    if (ret != 1) {
        LOG_FATAL("InetAddress::inet_pton()");
    }
    addr_.sin_port = htons(port);
}

std::string InetAddress::toIp() const {
    char buf[INET_ADDRSTRLEN];
    const char* ret = inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    if (ret == nullptr) {
        buf[0] = '\0';
        LOG_ERROR("InetAddress::toIp::inet_ntop() error!");
    }
    return std::string(buf);
}

uint16_t InetAddress::toPort() const { 
    return ntohs(addr_.sin_port); 
}

std::string InetAddress::toIpPort() const {
    std::string ret = toIp();
    ret.push_back(':');
    return ret.append(std::to_string(toPort()));
}