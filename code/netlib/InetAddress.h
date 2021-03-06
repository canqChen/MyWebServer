#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <string>
#include <netinet/in.h>

class InetAddress {
public:
    InetAddress() = default;
    explicit
    InetAddress(uint16_t port, bool loopback = false);
    InetAddress(std::string_view ip, uint16_t port);

    void setAddress(const struct sockaddr_in& addr) { 
        addr_ = addr; 
    }

    const struct sockaddr* getSockaddr() const { 
        return reinterpret_cast<const struct sockaddr*>(&addr_); 
    }

    socklen_t getSocklen() const { 
        return sizeof(addr_); 
    }

    std::string toIp() const;
    uint16_t toPort() const;
    std::string toIpPort() const;

private:
    struct sockaddr_in addr_;
};


#endif //INETADDRESS_H
