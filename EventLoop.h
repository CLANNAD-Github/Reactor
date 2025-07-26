#pragma once
#include "Epoll.h"
#include <functional>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <memory>
#include <map>
#include <atomic>

class Connection;
using spConnection = std::shared_ptr<Connection>;

class EventLoop
{
private:
    Epoll* m_ep = nullptr;                              // 一个事件对应一个 Epol 对象
    std::function<void()> m_epolltimeout_callbackfn;    // 关闭链接的回调函数 Channel 对象时就指定
    pid_t m_threadid;                                   // 事件循环所在的线程id
    std::queue<std::function<void()>> m_task_queue;     // 事件循环的任务队列
    std::mutex m_mutex;                                 // 操作任务队列的互斥锁
    int m_eventfd;                                      // 事件循环的事件 FD
    std::unique_ptr<Channel> m_event_channel;           // 事件循环的事件 Channel，注册 事件循环的 事件 FD，从而监听该FD上的事件
    int m_timerfd;                                      // 事件循环的定时 FD
    std::unique_ptr<Channel> m_timer_channel;           // 事件循环的定时 Channel，注册 事件循环的 时间 FD，从而监听该FD上的超时事件
    bool m_ismainloop;                                  // 表示该循环是否为主事件循环
    std::map<int, spConnection> m_con;                  // 存放所有在该事件循环对象上的所有 Connection
    std::function<void(int)> m_erase_connection_callbackfn;     // 删除 TcpServer 中的超时 Connection 的回调函数成员，在 Connection 超时时回调
    std::atomic_bool m_flag_stop;

public:
    EventLoop(Epoll * ep, bool ismainloop);
    ~EventLoop();

    Epoll* get_ep();
    void loop();
    void stop();
    void update_channel(Channel * ch);
    void remove_channel(Channel * ch);
    void set_epolltimeout_callbackfn(std::function<void()> fn);
    void set_erase_connection_callbackfn(std::function<void(int)> fn);

    bool is_iniothread();
    void enqueue(std::function<void()> fn);

    void wakeup();
    void handle_wakeup();
    void handle_timer();

    void add_Connection(spConnection con);
};

