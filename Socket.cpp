#include "Socket.h"

int create_nonblock_listenfd()
{
   int listen_fd = socket(AF_INET, SOCK_STREAM | O_NONBLOCK, IPPROTO_TCP);
   if (listen_fd < 0)
   {
        perror("socket() error\n");
        // printf("%s,%s,%d:create socket faild.\n", __FILE__, __FUNCTION__, errno);
        // 创建监听的 socket 失败，直接退出程序
        exit(-1);
   }
   return listen_fd;
}

Socket::Socket(int fd) : m_fd(fd)
{}

Socket::Socket(int fd, const std::string& client_ip, uint16_t client_port) : m_fd(fd), m_ip(client_ip), m_port(client_port)
{}

Socket::~Socket()
{
    close(m_fd);
}

int Socket::get_fd() const
{
    return m_fd;
}

std::string Socket::get_ip() const
{
    return m_ip;
}

uint16_t Socket::get_port() const
{
    return m_port;
}

void Socket::set_reuseaddr(bool flag)
{
    if (flag)
    {
        int opt = 1;
        setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt))); 
    }
}

void Socket::set_reuseport(bool flag)
{
    if (flag)
    {
        int opt = 1;
        setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt))); 
    }
}

void Socket::set_keepalive(bool flag)
{
    if (flag)
    {
        int opt = 1;
        setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt))); // 发送连接活跃心跳报文，维持长连接
    }
}

void Socket::set_tcpnodelay(bool flag)
{
    if (flag)
    {
        int opt = 1;
        setsockopt(m_fd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));  // 禁用 Nagle 算法
    }
}


void Socket::bind(const InetAddress& server_addr)
{
    if (::bind(m_fd, server_addr.addr(), sizeof(struct sockaddr)) < 0)
    {
        perror("bind() error.\n");
        exit(-1);
    }
    m_ip = server_addr.get_ip();
    m_port = server_addr.get_port();
}

void Socket::listen(int backlog)
{
    if (::listen(m_fd, backlog) != 0)
    {
        perror("listen() faild.\n");
        exit(-1);
    }
}

int Socket::accept(InetAddress& client_addr)
{
    struct sockaddr_in peer_addr;
    socklen_t len = sizeof(peer_addr);
    int client_fd = accept4(m_fd, (struct sockaddr*)&peer_addr, (socklen_t*)&len, O_NONBLOCK);
    client_addr.set_addr(peer_addr);
    return client_fd;
}
