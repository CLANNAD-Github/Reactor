#pragma once
#include <memory>
#include "EventLoop.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <string>
#include <map>

// 网络服务类，一个网络服务类中有一个 事件循环成员，单线程程序中只有一个，多线程程序中可以有多个
class TcpServer
{
private:
    std::unique_ptr<EventLoop> m_evloop;    // 主事件循环，一个 TcpServer 只有一个事件循环，因此使用 unique_ptr 表示独占该对象
    std::vector<std::unique_ptr<EventLoop>> m_subevloops;   // 从事件循环，可以有多个，每一个都是唯一的
    int m_thread_num;                       // 指定线程池中的线程数量
    std::unique_ptr<ThreadPool> m_tp;       // 线程池，用于运行从事件循环，每一个线程运行一个从事件循环
    Acceptor m_acceptor;                    // 接收器，用于接受客户端连接，一个服务器只有一个接收器
    std::map<int, spConnection> m_map_con;  // 链接容器存储每一个 connection 链接
    std::mutex m_mutex;                     // m_map_con 的互斥锁

    std::function<void (std::shared_ptr<Socket>)> m_handle_connection_callback;
    std::function<void (spConnection)> m_close_connection_callback;
    std::function<void (spConnection)> m_error_connection_callback;
    std::function<void (spConnection, std::shared_ptr<std::string>)> m_handle_message_callback;
    std::function<void (spConnection)> m_send_complete_callback;
    std::function<void ()> m_epoll_timeout_callback;
public:
    TcpServer(const std::string & ip, const uint32_t port, int thread_num);
    ~TcpServer();

    void start();
    void stop();

    void handle_connection(std::shared_ptr<Socket> client_socket);
    void close_connection(spConnection con);
    void error_connection(spConnection con);
    void handle_message(spConnection con, std::shared_ptr<std::string>); // 服务端处理消息的函数（回调 Connection 类中的函数）
    void send_complete(spConnection hcon);
    void epoll_timeout();
    void erase_connection(int fd); // 根据 fd 删除 Connection 对象。供 EventLoop 对象回调

    void set_handle_connection_callback(std::function<void (std::shared_ptr<Socket>)> fn);
    void set_close_connection_callback(std::function<void (spConnection)> fn);
    void set_error_connection_callback(std::function<void (spConnection)> fn);
    void set_handle_message_callback(std::function<void (spConnection, std::shared_ptr<std::string>)> fn);
    void set_send_complete_callback(std::function<void (spConnection)> fn);
    void set_epoll_timeout_callback(std::function<void ()> fn);
};
