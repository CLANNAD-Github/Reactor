#pragma once
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

class InetAddress
{
private:
    sockaddr_in m_addr;
public:
    InetAddress();
    // 创建服务端的socket
    InetAddress(const std::string& ip, uint16_t port);
    // 创建客户端连接上来的socket
    InetAddress(const sockaddr_in addr);
    ~InetAddress();

    const char* get_ip() const;
    uint16_t get_port() const;
    const sockaddr* addr() const;
    void set_addr(const sockaddr_in addr);
};

