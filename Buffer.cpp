#include "Buffer.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

Buffer::Buffer(enum data_type type) : m_type(type)
{}

Buffer::~Buffer()
{}

bool Buffer::empty() const { return m_data.empty(); }
size_t Buffer::size() const { return m_data.size(); }
void Buffer::erase(int pos, int size) { m_data.erase(pos, size); }
void Buffer::clear() { m_data.clear(); }

void Buffer::append(const char* data, size_t size)
{
    switch (m_type)
    {
    case data_type::T_NONE:
    {
        m_data.append(data, size);
        break;
    }
    case data_type::T_INT_HEAD:
    {
        m_data.append((char*)&size, sizeof(size));
        m_data.append(data, size);
        break;
    }
    default:
        break;
    }
}

bool Buffer::pick_message(std::string& message)
{
    if (m_data.size() < 4)
        return false;
    int size;
    memcpy(&size, m_data.data(), sizeof(int));
    if (size > m_data.size()-4)
        return false;
    m_data.erase(0, 4);
    message.clear();
    message.append(m_data.substr(0, size));
    m_data.erase(0, size);
    return true;
}

int Buffer::recv_from_fd(int fd)
{
    char buffer[4096];
    int nread = read(fd, buffer, sizeof(buffer));

    if (nread <= 0)
        return nread;
    
    m_data.append(buffer, nread);
    return nread;
}

int Buffer::send_to_fd(int fd)
{
    int res = send(fd, m_data.data(), m_data.size(), 0);
    if (res > 0)
        m_data.erase(0, res);
    return res;
}
