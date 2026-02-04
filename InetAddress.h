#pragma once

#include <arpa/inet.h>
#include <string>

class InetAddress
{
private:
    struct sockaddr_in m_addr;
public:
    InetAddress();
    InetAddress(struct sockaddr_in addr);
    InetAddress(const std::string& ip, uint16_t port);
    ~InetAddress();

    void setaddr(struct sockaddr_in addr);
    const sockaddr* addr() const;
    const char* ip()const;
    uint16_t port() const;
};
