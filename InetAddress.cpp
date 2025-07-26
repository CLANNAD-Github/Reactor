#include "InetAddress.h"

InetAddress::InetAddress() {}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const sockaddr_in addr) : m_addr(addr) {}

InetAddress::~InetAddress() {}

const char* InetAddress::get_ip() const
{
    return inet_ntoa(m_addr.sin_addr);
}

uint16_t InetAddress::get_port() const
{
    return m_addr.sin_port;
}

const sockaddr* InetAddress::addr() const
{
    return (struct sockaddr*)&m_addr;
}

void InetAddress::set_addr(const sockaddr_in addr)
{
    m_addr = addr;
}

