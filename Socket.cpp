#include "Socket.h"
#include <fcntl.h>

Socket::Socket(const int fd, const std::string& ip, uint16_t port)
 : m_fd(fd), m_ip(ip), m_port(port)
{}

int Socket::fd() const { return m_fd; }
std::string Socket::ip() const { return m_ip; }
uint16_t Socket::port() const { return m_port; }

void Socket::setnonblock() { fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL) | O_NONBLOCK); }

void Socket::setreuseaddr()
{
    int opt = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));
}
void Socket::setnodelay()
{
    int opt = 1;
    setsockopt(m_fd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));
}
void Socket::setreuseport()
{
    int opt = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));
}
void Socket::setkeepalive()
{
    int opt = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));
}

bool Socket::bind(const InetAddress server_addr)
{
    if (::bind(m_fd, server_addr.addr(), static_cast<socklen_t>(sizeof(struct sockaddr_in))) < 0)
    {
        fprintf(stderr, "%s %d %d: bind error.", __FILE__, __LINE__, errno);
        exit(EXIT_FAILURE);
    }
    return true;
}

bool Socket::listen(int backlog)
{
    
    if (::listen(m_fd, backlog) != 0)
    {
        fprintf(stderr, "%s %d %d: listen error.", __FILE__, __LINE__, errno);
        exit(EXIT_FAILURE);
    }
    return true;
}

int Socket::accept(InetAddress& client_addr)
{
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);

    int client_fd = ::accept(m_fd, reinterpret_cast<struct sockaddr*>(&peer_addr), &peer_addr_len);
    client_addr.setaddr(peer_addr);

    if (client_fd < 0)
    {
        fprintf(stderr, "%s %d %d: accept error.", __FILE__, __LINE__, errno);
        return client_fd;
    }
    return client_fd;
}

Socket::~Socket()
{
    close(m_fd);
}
