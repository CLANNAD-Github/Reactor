#pragma once

class Epoll;
class Channel;

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <thread>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <sys/timerfd.h>
#include <map>
#include <memory>
#include <atomic>

class Channel;
class Connection;

class EventLoop
{
    using spConnection = std::shared_ptr<Connection>;
private:

    std::unique_ptr<Epoll> m_epoll;
    std::function<void (EventLoop *)> m_timeout_callback_fn;
    std::function<void (spConnection)> m_remove_connection_callback_fn;

    pid_t m_thread_id; // 标识该事件循环的 线程ID，便于后面进行判断
    std::queue<std::function<void ()>> m_send_queue; // 事件循环的发送队列，Connection 的发送任务都存放于此，事件循环负责取出并发送
    std::mutex m_mutex_send_queue;

    int m_eventfd;
    std::unique_ptr<Channel> m_event_channel; // 事件循环 的Channel，用于通知 EventLoop 对象处理 event 事件
    int m_timerfd;
    std::unique_ptr<Channel> m_timer_channel; // 事件循环 的闹钟 Channel，用于通知 EventLoop 对象处理 event 事件

    std::mutex m_mutex_map_con;
    std::map<int, spConnection> m_map_con;
    int m_timeout_con;  // 客户端连接的超时时间
    int m_timer;        // 定时器的间隔时间
    bool m_ismainloop;
    std::atomic<bool>  m_stop_flag; // 原子类型的停止循环标志

public:
    EventLoop(bool ismainloop, int timer = -1, int timeout_con = -1);
    ~EventLoop();

    pid_t thread_id() const;

    void run(int epoll_timeout);
    void stop();

    Epoll * epoll() const;
    void update_channel(Channel * ch);
    void remove_channel(Channel * ch);

    void push_queue(std::function<void ()> fn);
    void handle_eventfd();
    void add_connection(spConnection con); // TcpServer 调用该函数添加新建的 Connection 链接
    void remove_connection(spConnection con); // TcpServer 调用该函数添加新建的 Connection 链接
    void handle_timerfd();

    void set_timeout_callback_fn(std::function<void (EventLoop *)> fn);
    void set_remove_connection_callback_fn(std::function<void (spConnection)> fn);
};
