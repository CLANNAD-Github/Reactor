
#include "Channel.h"
#include "Socket.h"
#include "Connection.h"
#include <string.h>

Channel::Channel(EventLoop* evloop, int fd) : m_evloop(evloop), m_fd(fd)
{}

// 析构函数什么也不做，因为 fd 、 epoll 都是从外部传递而来，并不属于 Channel 本身
Channel::~Channel()
{}

int Channel::get_fd() const
{
    return m_fd;
}

void Channel::use_ET()
{
    m_event = m_event | EPOLLET;
}

void Channel::enable_read()
{
    m_event = m_event | EPOLLIN;
    m_evloop->update_channel(this);
}

void Channel::enable_write()
{
    m_event = m_event | EPOLLOUT;
    m_evloop->update_channel(this);
}

void Channel::disable_read()
{
    m_event = m_event & (~EPOLLIN);
    m_evloop->update_channel(this);
}

void Channel::disable_write()
{
    m_event = m_event & (~EPOLLOUT);
    m_evloop->update_channel(this);
}

void Channel::disable_all()
{
    m_event = 0;
    m_evloop->update_channel(this);
}

void Channel::remove()
{
    m_evloop->remove_channel(this);
}

void Channel::set_inepoll()
{
    m_inepoll = true;
}

void Channel::set_revent(uint32_t ev)
{
    m_revent = ev;
}

bool Channel::in_epoll() const
{
    return m_inepoll;
}

uint32_t Channel::get_event() const
{
    return m_event;
}

uint32_t Channel::get_revent() const
{
    return m_revent;
}

void Channel::handle_event()
{
    if (m_revent & (EPOLLRDHUP)) // 对方已关闭，检测不到，有些系统可能不支持该事件
    {
        m_close_callbackfn();
    }
    else if (m_revent & (EPOLLIN | EPOLLPRI)) // 处理读事件，带外数据
    {
        m_read_callbackfn();
    }
    else if (m_revent & EPOLLOUT) // 处理写事件
    {
        m_write_callbackfn();
    }
    else // 发生错误
    {
        m_error_callbackfn();
    }
}

void Channel::set_read_callbackfn(std::function<void()> fn)
{
    m_read_callbackfn = fn;
}

void Channel::set_write_callbackfn(std::function<void ()> fn)
{
    m_write_callbackfn = fn;
}

void Channel::set_close_callbackfn(std::function<void()> fn)
{
    m_close_callbackfn = fn;
}

void Channel::set_error_callbackfn(std::function<void()> fn)
{
    m_error_callbackfn = fn;
}
