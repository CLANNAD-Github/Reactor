#pragma once

#include <functional>

typedef unsigned int uint32_t;

class EventLoop;
class Socket;
class Acceptor;

class Channel
{
private:
    int m_fd;
    EventLoop * m_eventloop;
    uint32_t m_event;
    uint32_t m_revent;
    bool m_inepoll;
    bool m_islisten;

    std::function<void ()> m_read_callback_fn;
    std::function<void ()> m_write_callback_fn;
    std::function<void ()> m_error_callback_fn;
    std::function<void ()> m_close_callback_fn;

public:
    Channel(EventLoop * eventloop, int fd, bool islisten = false);
    ~Channel();

    int fd() const;
    bool inepoll() const;
    void set_inepoll(bool inepoll);
    void set_event(uint32_t event);
    void set_revent(uint32_t revent);
    uint32_t event() const;
    uint32_t revent() const;
    bool islisten() const;

    void enable_et();
    void enable_read();
    void enable_write();

    void disable_et();
    void disable_read();
    void disable_write();
    void update_channel();

    void handle_event();

    void set_read_callback_fn(std::function<void ()> fn);
    void set_write_callback_fn(std::function<void ()> fn);
    void set_error_callback_fn(std::function<void ()> fn);
    void set_close_callback_fn(std::function<void ()> fn);
};

