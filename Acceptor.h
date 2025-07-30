#pragma once
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Connection.h"
#include <functional>

class Acceptor
{
private:
    // 注意初始化顺序
    EventLoop* m_evloop;        // EventLoop 类对象是从外部传进来的，并不属于 Acceptor ，因此可以使用裸指针，不必使用智能指针或引用。引用裸指针更方便
    Socket m_server_socket;   // socket 和 channel 都属于 Acceptor 类，可以使用栈内存，在创建对象时自动分配
    Channel m_accept_channal; // 且这两个成员占用空间都不大
    // 回调函数成员，回调TcpServer 类中的 newconnection 函数创建新的客户端链接
    std::function<void (std::shared_ptr<Socket>)> m_handle_connection_callbackfn;
public:
    Acceptor(EventLoop * evloop, const std::string & ip, const uint32_t port);
    ~Acceptor();

    void handle_connection();
    void set_handle_connection_callbackfn(std::function<void (std::shared_ptr<Socket>)> fn);
};
