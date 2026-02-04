#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <sys/eventfd.h>
#include "Connection.h"

EventLoop::EventLoop(bool ismainloop, int timer, int timeout_con)
    : m_epoll(new Epoll()),
    m_eventfd(eventfd(0, EFD_NONBLOCK)),
    m_event_channel(new Channel(this, m_eventfd)),
    m_timerfd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)),
    m_timer_channel(new Channel(this, m_timerfd)),
    m_timer(timer),
    m_timeout_con(timeout_con),
    m_ismainloop(ismainloop),
    m_stop_flag(false)
{
    // 只有传入正确的时间值才能开启定时器，监听定时器事件
    if (m_timer > 0)
    {        
        struct itimerspec timer_spec;
        timer_spec.it_value.tv_sec = m_timer;
        timer_spec.it_value.tv_nsec = 0;
        timer_spec.it_interval.tv_sec = m_timer;
        timer_spec.it_interval.tv_nsec = 0;
        timerfd_settime(m_timerfd, 0, &timer_spec, nullptr);
        m_timer_channel->enable_read();
        m_timer_channel->set_read_callback_fn(std::bind(&EventLoop::handle_timerfd, this));
    }
 
    m_event_channel->enable_read();
    m_event_channel->set_read_callback_fn(std::bind(&EventLoop::handle_eventfd, this));
}

EventLoop::~EventLoop()
{}

Epoll * EventLoop::epoll() const { return m_epoll.get(); }
void EventLoop::update_channel(Channel * ch) { m_epoll->update_channel(ch); }
void EventLoop::remove_channel(Channel * ch) { m_epoll->remove_channel(ch); }

pid_t EventLoop::thread_id() const { return m_thread_id; }

void EventLoop::push_queue(std::function<void ()> fn)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex_send_queue);
        m_send_queue.push(fn);
    }

    uint64_t data = 1;
    write(m_eventfd, &data, sizeof(data));
}

void EventLoop::handle_eventfd()
{
    if (m_ismainloop)
    {
        uint64_t buffer;
        read(m_eventfd, &buffer, sizeof(buffer));
    }
    else
    {
        uint64_t buffer;
        read(m_eventfd, &buffer, sizeof(buffer));
        
        std::function<void ()> fn;
        while (!m_send_queue.empty())
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex_send_queue);
                fn = m_send_queue.front();
                m_send_queue.pop();
            }
            fn();
        }
    }    
}

// 当新的连接到达是，TcpServer 收到连接请求，然后添加到对应的子事件循环的Connection map 容器
// 该函数运行在主线程中，与子线程中的删除 Connection 会产生资源竞争与冲突，因此需要加锁
void EventLoop::add_connection(spConnection con)
{
    std::lock_guard<std::mutex> lock(m_mutex_map_con);
    m_map_con.emplace(con->fd(),con);
}

void EventLoop::remove_connection(spConnection con)
{
    std::lock_guard<std::mutex> lock(m_mutex_map_con);
    m_map_con.erase(con->fd());
}

void EventLoop::handle_timerfd()
{
    if (m_ismainloop)
    {
        uint64_t expirations;
        ssize_t s = read(m_timerfd, &expirations, sizeof(expirations));
    }
    else
    {
        uint64_t expirations;
        ssize_t s = read(m_timerfd, &expirations, sizeof(expirations));

        for (auto iter = m_map_con.cbegin(); iter != m_map_con.cend();)
        {
            // 判断是否已经断连接
            if (iter->second->disconnect())
            {
                printf("Connection FD %d disconnect time:%d\n", iter->first, iter->second->time_stamp().to_time(), time(NULL));
                std::lock_guard<std::mutex> lock(m_mutex_map_con);
                iter = m_map_con.erase(iter);
            }
            // 判断是否超时
            else if (time(NULL) - iter->second->time_stamp().to_time() > m_timeout_con)
            {
                printf("Connection FD %d timeout. Connection timestamp:%d now:%d\n", iter->first, iter->second->time_stamp().to_time(), time(NULL));
                m_remove_connection_callback_fn(iter->second);
                {
                    std::lock_guard<std::mutex> lock(m_mutex_map_con);
                    iter = m_map_con.erase(iter);
                }
            }
            else
                iter++;
        }
    }
}

void EventLoop::run(int epoll_timeout)
{
    m_thread_id = syscall(SYS_gettid);

    std::vector<Channel*> vec_ch;

    while (!m_stop_flag)
    {
        vec_ch = m_epoll->loop(epoll_timeout);

        if (vec_ch.empty())
        {
            if (m_timeout_callback_fn)
                m_timeout_callback_fn(this);
            continue;
        }
        for (auto e : vec_ch)
        {
            e->handle_event();
        }
    }
}

void EventLoop::stop()
{
    if (m_stop_flag == false)
    {
        m_stop_flag = true;

        if(!m_ismainloop)
        {
            uint64_t data = 1;
            write(m_eventfd, &data, sizeof(data));
        }
    }
}

void EventLoop::set_timeout_callback_fn(std::function<void (EventLoop*)> fn) { m_timeout_callback_fn = fn; }
void EventLoop::set_remove_connection_callback_fn(std::function<void (spConnection)> fn) { m_remove_connection_callback_fn = fn; }
