#pragma once
#include <unistd.h>
#include "InetAddress.h"
#include <sys/socket.h>
#include <sys/fcntl.h>

int create_nonblock_listenfd();

class Socket
{
private:
    // 一个 socket 对应一个 fd，因此声明为 const，根据创建时传入的 fd 确定，之后不再改变
    const int m_fd;
    std::string m_ip;   // socket 自身的 IP 地址，可能时服务端也可能时客户端
    uint16_t m_port;    // socket 自身对应的端口
public:
    Socket(int fd);
    // 客户端链接上来时调用，创建 客户端的 socket
    Socket(int fd, const std::string& client_ip, uint16_t client_port);
    ~Socket();

    int get_fd()const;
    std::string get_ip()const;
    uint16_t get_port()const;

    // 设置 socket 选项，flag 为 true 时开启
    void set_reuseaddr(bool flag);
    void set_reuseport(bool flag);
    void set_tcpnodelay(bool flag);
    void set_keepalive(bool flag);

    void bind(const InetAddress& server_addr);
    void listen(int backlog = 128);
    int accept(InetAddress& client_addr);
};
