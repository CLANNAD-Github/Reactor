#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "InetAddress.h"

class Socket
{
private:
    const int m_fd;
    std::string m_ip;
    uint16_t m_port;

public:
    Socket(const int fd, const std::string& ip, uint16_t port);
    ~Socket();

    void setnonblock();
    void setreuseaddr();
    void setnodelay();
    void setreuseport();
    void setkeepalive();

    int fd() const;
    std::string ip() const;
    uint16_t port() const;

    bool bind(InetAddress server_addr);
    bool listen(int backlog = 128);
    int accept(InetAddress& client_addr);
};
