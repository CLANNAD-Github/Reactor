
#include "Buffer.h"
#include <cstring>

Buffer::Buffer(const int sep) : m_sep(sep)
{}

Buffer::~Buffer()
{}

void Buffer::append(const char* data, size_t size)
{
    m_data.append(data, size);
}

void Buffer::append_with_sep(const char* data, int size)
{
    if (m_sep == 0)
    {
        m_data.append(data, size);
    }
    else if (m_sep == 1)
    {
        m_data.append((char*)&size, sizeof(int));
        m_data.append(data, size);
    }
    else if (m_sep == 2)
    {
        m_data.append("\r\n\r\n", 4);
        m_data.append(data, size);
    }
}

size_t Buffer::size() const
{
    return m_data.size();
}

const char* Buffer::data() const
{
    return m_data.data();
}

void Buffer::clear()
{
    m_data.clear();
}

void Buffer::erase(int pos, int size)
{
    m_data.erase(pos, size);
}

bool Buffer::pickmessage(std::string& message)
{
    if (m_data.empty())
        return false;
    
    if (m_sep == 0)
    {
        message = m_data;
    }
    else if(m_sep == 1)
    {
        int len;
        memcpy(&len, m_data.c_str(), sizeof(int));
        if (m_data.size() >= sizeof(int) + len)
        {
            message = m_data.substr(sizeof(int), len);
            m_data.erase(0, len + sizeof(int));
        }
    }
    return true;
}
