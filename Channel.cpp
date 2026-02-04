#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>

Channel::Channel(EventLoop * eventloop, int fd, bool islisten)
    : m_eventloop(eventloop), m_fd(fd), m_event(0), m_revent(0), m_islisten(islisten), m_inepoll(false)
{}

Channel::~Channel()
{
    m_eventloop->remove_channel(this);
}

int Channel::fd() const { return m_fd; }
bool Channel::islisten() const { return m_islisten; }

bool Channel::inepoll() const { return m_inepoll; }
void Channel::set_inepoll(bool inepoll) { m_inepoll = inepoll; }
uint32_t Channel::event() const { return m_event; }
void Channel::set_event(uint32_t event) { m_event = event; }
uint32_t Channel::revent() const { return m_revent; }
void Channel::set_revent(uint32_t revent) { m_revent = revent; }

void Channel::enable_read()
{
    m_event |= EPOLLIN;
    update_channel();
}

void Channel::enable_write()
{
    m_event |= EPOLLOUT;
    update_channel();
}

void Channel::enable_et()
{
    m_event |= EPOLLET;
    update_channel();
}

void Channel::disable_read()
{
    m_event &= (~EPOLLIN);
    update_channel();
}

void Channel::disable_write()
{
    m_event &= (~EPOLLOUT);
    update_channel();
}

void Channel::disable_et()
{
    m_event &= (~EPOLLET);
    update_channel();
}

void Channel::update_channel() { m_eventloop->update_channel(this); }

void Channel::handle_event()
{
    if (m_revent & EPOLLHUP)
    {
        if (m_close_callback_fn)
            m_close_callback_fn();
    }
    else if (m_revent & (EPOLLIN | EPOLLPRI))
    {
        if(m_read_callback_fn)
            m_read_callback_fn();
    }
    else if (m_revent & EPOLLOUT)
    {
        if (m_write_callback_fn)
            m_write_callback_fn();        
    }
    else
    {
        if (m_error_callback_fn)
            m_error_callback_fn();
    }
}

void Channel::set_read_callback_fn(std::function<void ()> fn) { m_read_callback_fn = fn; }
void Channel::set_write_callback_fn(std::function<void ()> fn) { m_write_callback_fn = fn; }
void Channel::set_error_callback_fn(std::function<void ()> fn) { m_error_callback_fn = fn; }
void Channel::set_close_callback_fn(std::function<void ()> fn) { m_close_callback_fn = fn; }