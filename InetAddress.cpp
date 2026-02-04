#include "InetAddress.h"

InetAddress::InetAddress() {}
InetAddress::InetAddress(struct sockaddr_in addr) { m_addr = addr; }

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    m_addr.sin_port = htons(port);
}

void InetAddress::setaddr(struct sockaddr_in addr) { m_addr = addr; }
const sockaddr* InetAddress::addr() const { return (const sockaddr*)&m_addr; }
const char* InetAddress::ip() const { return inet_ntoa(m_addr.sin_addr); }
uint16_t InetAddress::port() const { return m_addr.sin_port; }

InetAddress::~InetAddress()
{}
