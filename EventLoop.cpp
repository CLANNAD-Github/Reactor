#include "EventLoop.h"
#include "Channel.h"
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "Connection.h"
#include <set>

static int create_timerfd()
{
    // 创建时间 fd
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    // 指定超时时间
    struct itimerspec new_value = {0};
    new_value.it_value.tv_sec = 5;
    new_value.it_value.tv_nsec = 0;
    timerfd_settime(timer_fd, 0, &new_value, 0);
    return timer_fd;
}

EventLoop::EventLoop(Epoll * ep, bool ismainloop) : m_ep(ep), m_ismainloop(ismainloop), m_flag_stop(false),
    m_eventfd(eventfd(0, EFD_NONBLOCK)), m_event_channel(new Channel(this, m_eventfd)),
    m_timerfd(create_timerfd()), m_timer_channel(new Channel(this, m_timerfd))
{
    // 注册事件 Channel 的写事件和回调
    m_event_channel->set_read_callbackfn(std::bind(&EventLoop::handle_wakeup, this));
    m_event_channel->enable_read();
    m_timer_channel->set_read_callbackfn(std::bind(&EventLoop::handle_timer, this));
    m_timer_channel->enable_read();
}

EventLoop::~EventLoop()
{
    delete m_ep;
    printf("~EventLoop()...ismainloop(%s) threadid(%d) m_threadid(%d)\n", (m_ismainloop?"true":"false"), syscall(SYS_gettid), m_threadid);
}

Epoll* EventLoop::get_ep()
{
    return m_ep;
}

void EventLoop::loop()
{
    // 因为 事件循环在 TcpServer 中创建，然后在 IO 线程中运行，所以应该在 该函数中获取线程 ID，此时线程 ID 必定属于 IO 线程
    m_threadid = syscall(SYS_gettid);
    std::vector<Channel *> vec_ch;
    while (m_flag_stop != true)
    {
        vec_ch = m_ep->loop(5 * 1000);
        // vec_ch = m_ep->loop();
        // 返回空容器，说明 epoll 超时，回调 tcpserver 的超时函数
        if (vec_ch.empty())
        {
            m_epolltimeout_callbackfn();
        }
        else
        {
            for (auto & ch : vec_ch)
            {
                ch->handle_event();
            }
        }
    }
}

void EventLoop::stop()
{
    if (m_flag_stop == true)
        return;
    
    // 设置停止标志后，立即唤醒事件循环，让 事件循环 结束 epoll_wait 的阻塞状态，从而退出事件循环
    m_flag_stop = true;
    // 唤醒事件循环后，会执行所有线程中的函数，执行完之后，事件循环才会完成退出，这样就完成了退出前的收尾工作
    wakeup();
}

void EventLoop::update_channel(Channel * ch)
{
    m_ep->update_channel(ch);
}

void EventLoop::remove_channel(Channel * ch)
{
    m_ep->remove_channel(ch);
}

void EventLoop::set_epolltimeout_callbackfn(std::function<void()> fn)
{
    m_epolltimeout_callbackfn = fn;
}

void EventLoop::set_erase_connection_callbackfn(std::function<void(int)> fn)
{
    m_erase_connection_callbackfn = fn;
}

bool EventLoop::is_iniothread()
{
    // 调用该函数的地方获取的线程，与之前在 IO 线程运行时获取的线程进行比较，然后就可以判断出该线程是否为 IO 线程
    return m_threadid == syscall(SYS_gettid);
}

void EventLoop::enqueue(std::function<void()> fn)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task_queue.push(fn);
    }
    // 放入任务队列中后，唤醒事件循环
    wakeup();
}

void EventLoop::wakeup()
{
    // 向 FD 中写入数据即可出发写事件，然后就可以回调 handle_wakeup 函数进行处理
    uint64_t val = 1;
    write(m_eventfd, &val, sizeof(val));
}

void EventLoop::handle_wakeup()
{
    printf("handle_wakeup() called in thread %d\n", syscall(SYS_gettid));
    uint64_t val;
    std::function<void()> fn;
    read(m_eventfd, &val, sizeof(val));

    std::lock_guard<std::mutex> lock(m_mutex);

    // 唤醒一次，执行队列中所有的任务
    while (m_task_queue.size() > 0)
    {
        fn = std::move(m_task_queue.front());
        m_task_queue.pop();
        fn();
    }
}

void EventLoop::handle_timer()
{
    // 超时调用该函数，重新设置超时时间
    struct itimerspec new_value = {0};
    new_value.it_value.tv_sec = 5;
    new_value.it_value.tv_nsec = 0;
    timerfd_settime(m_timerfd, 0, &new_value, 0);
    if (m_ismainloop)
    {
        // printf("闹钟时间到，主事件循环（ThreadID：%d）执行函数。。。\n", syscall(SYS_gettid));
    }
    else
    {
        // printf("闹钟时间到，从事件循环（ThreadID：%d）执行函数。。。\n", syscall(SYS_gettid));
        // 只有从事件循环才有 FD 需要进行判断，超时判读属于 Connection 的操作，因此在这里调用 Connection 的超时判断方法
        //printf("ThreadID:%d FD:", syscall(SYS_gettid));
        time_t now = time(NULL);
        
        for (auto it = m_con.begin(); it != m_con.end(); it++)
        {
            printf("%d ", it->first);
            if (it->second->istimeout(now, 10))
            {
                printf("已超时. ");
                // Connection 超时，不仅要删除 EventLoop 中的 Connection 指针，还需要删除 TcpServer 中的指针
                // EventLoop 中没有 TcpServer 对象，而是 TcpServer 中包含 EventLoop 对象，因此可以采用回调函数的方式回调删除 Connection 的函数
                m_erase_connection_callbackfn(it->first);
                it = m_con.erase(it);
            }
        }
        putchar('\n');
    }
}

void EventLoop::add_Connection(spConnection con)
{
    m_con[con->get_fd()] = con;
}
