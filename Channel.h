#pragma once
#include <memory>
#include "EventLoop.h"

class Socket;

class Channel
{
private:
    int m_fd = -1;                      // 一个 Channel 对应一个 socket fd
    EventLoop * m_evloop;               // 事件循环，一个 Channel 对应一个 事件循环
    bool m_inepoll = false;             // Channel 是否已经在 epoll 红黑树中
    uint32_t m_event = 0;               // 监听的事件
    uint32_t m_revent = 0;              // 返回的事件
    std::function<void()> m_read_callbackfn;  // 读取数据的回调函数 Channel 对象时就指定
    std::function<void()> m_write_callbackfn;  // 读取数据的回调函数 Channel 对象时就指定
    std::function<void()> m_close_callbackfn;  // 关闭链接的回调函数 Channel 对象时就指定
    std::function<void()> m_error_callbackfn;  // 发生错误的回调函数 Channel 对象时就指定
public:
    Channel(EventLoop* evloop, int fd);
    ~Channel();

    int get_fd() const;
    uint32_t get_event() const;
    uint32_t get_revent() const;

    void use_ET();
    void enable_read();   // 注册写事件
    void enable_write();   // 注册读事件
    void disable_write();  // 取消关注写事件
    void disable_read();  // 取消关注读事件
    void disable_all();  // 取消关注全部的事件
    void remove();  // 将 channel 从 epoll 红黑树中删除
    void set_inepoll();
    void set_revent(uint32_t ev);
    bool in_epoll() const;
    
    void handle_event();
    void set_read_callbackfn(std::function<void ()> fn);
    void set_write_callbackfn(std::function<void ()> fn);
    void set_close_callbackfn(std::function<void ()> fn);
    void set_error_callbackfn(std::function<void ()> fn);
};

