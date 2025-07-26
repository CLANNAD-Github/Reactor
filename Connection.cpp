#include "Connection.h"
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <functional>

Connection::Connection(EventLoop* evloop, std::shared_ptr<Socket> client_socket) 
    : m_evloop(evloop), m_client_socket(client_socket), m_disconnect(false),
    m_inputbuffer(1),m_outputbuffer(1),
    m_client_channal(new Channel(m_evloop, m_client_socket->get_fd()))
{
    m_client_channal->enable_read();
    // m_client_channal->use_ET();
    m_client_channal->set_read_callbackfn(std::bind(&Connection::handle_message, this));
    m_client_channal->set_write_callbackfn(std::bind(&Connection::write_callback, this));
    m_client_channal->set_close_callbackfn(std::bind(&Connection::close_callback, this));
    m_client_channal->set_error_callbackfn(std::bind(&Connection::error_callback, this));
}

Connection::~Connection()
{
    printf("Connection destructor in Thread ID(%d)\n", syscall(SYS_gettid));
}

// 事件循环处理函数在 Channel 中进行，每一次回调都说明触发了写事件，也说明还有数据待发送，因此每一次回调都会发送数据
// 当数据发送完成后，会取消关注写事件，这样就会避免发送完了数据却仍然写事件的情况，从而避免了 busy loop
void Connection::write_callback()
{
    // printf("Connection::write_callback() in Thread ID(%d)\n", syscall(SYS_gettid));
    int nwrite = ::send(get_fd(), m_outputbuffer.data(), m_outputbuffer.size(), 0);
    // 发送多少数据就清除多少数据，即使发送失败，也不会误删除数据
    if (nwrite > 0)
        m_outputbuffer.erase(0, nwrite);
    // 尝试发送数据，如果发送完，就取消关注写事件
    if (m_outputbuffer.size() == 0)
    {
        m_client_channal->disable_write();
        // 回调 TcpServer 函数中的 sendcomplete() 函数表示数据发送完成
        m_sendcomplete_callbackfn(shared_from_this());
    }
}

// TcpSetver 类会处理接收的数据，处理完毕后，或者任意需要发送数据的时刻就会调用该函数，请求发送数据
// 该函数不直接发送数据，而是注册写事件，并将需要发送的数据
void Connection::send(std::shared_ptr<std::string> data)
{
    // 注意这里线程 ID 和上面 write_callback 函数的线程ID不在同一个，因为 采用了工作线程用于处理计算，避免在 IO 线程中因为计算过程出现阻塞
    // 但同时也带来一个问题，即：这两个函数都会操作 outputbuffer 输出缓冲区，会造成资源竞争。
    // 使用锁的开销很大，因为每一个链接都要有一把锁，但是 链接数目会很多，因此使用锁不是一个好方法
    // 可以采用 在IO线程中添加发送队列的方式来解决
    // printf("Connection::send() in Thread ID(%d)\n", syscall(SYS_gettid));
    if (m_disconnect == true)
    {
        // printf("Connection disconnected... send faild and return.\n");
        return;
    }

    if (m_evloop->is_iniothread())
    {
        // printf("send() 在事件循环中 threadid:%d\n", syscall(SYS_gettid));
        send_in_io_loop(data);
    }
    else
    {
        // printf("send() 不在事件循环中 threadid:%d\n", syscall(SYS_gettid));
        // 如果不在 IO 线程中，则加入到 IO 线程的任务队列中，让 IO 线程去执行
        m_evloop->enqueue(std::bind(&Connection::send_in_io_loop, this, data));
    }
}

void Connection::send_in_io_loop(std::shared_ptr<std::string> data)
{
    // 首先将数据加入到发送缓冲区中
    m_outputbuffer.append_with_sep(data->c_str(), data->size());
    // 然后注册写事件，供 Channel 类回调，每一次回调都会发送一部分数据，直至发送完所有数据，再取消关注写事件
    m_client_channal->enable_write();
}

void Connection::handle_message()
{
    // 客户端socket采用边缘触发的方式，因此需要循环读取数据
    char buffer[1024];
    while (true)
    {
        memset(buffer, '\0', sizeof(buffer));
        int nread = recv(get_fd(), buffer, sizeof(buffer), 0);
        if (nread > 0)
        {
            m_inputbuffer.append(buffer, nread);
            continue;
        }
        
        if (nread == -1 && errno == EINTR) // 不是必须的代码。但是更标准
        {
            // 如果被信号中断，继续读取
            continue;
        }
        if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) // 全部的数据已经读取完毕
        {
            std::shared_ptr<std::string> message(new std::string());
            while (true)
            {
                if (m_inputbuffer.pickmessage(*message) == false)
                    break;
                m_tsp = Timestamp::now();
                printf("Connection timestamp:(%s:%d fd:%d time:%s)\n",\
                    m_client_socket->get_ip().c_str(), m_client_socket->get_port(), m_client_socket->get_fd(), m_tsp.tostring().c_str());
                // 成功取出一条数据就交由上层类处理，使用回调函数
                m_handle_message_callbackfn(shared_from_this(), message);
            }
            break;
        }
        if (nread == 0)
        {
            // 如果客户端断开连接
            close_callback();
            break;
        }
    }
}

std::string Connection::get_ip() const
{
    return m_client_socket->get_ip();
}

uint16_t Connection::get_port() const
{
    return m_client_socket->get_port();
}

int Connection::get_fd() const
{
    return m_client_socket->get_fd();
}

void Connection::remove_channel()
{
    m_client_channal->remove();
}

void Connection::close_callback()
{
    m_disconnect = true;
    remove_channel();
    m_close_callbackfn(shared_from_this());
}

void Connection::error_callback()
{
    m_disconnect = true;
    remove_channel();
    m_error_callbackfn(shared_from_this());
}

bool Connection::istimeout(time_t t, int val) const
{
    return (t - m_tsp.toint()) > val;
}

void Connection::set_close_callbackfn(std::function<void(spConnection)> fn)
{
    m_close_callbackfn = fn;
}

void Connection::set_error_callbackfn(std::function<void(spConnection)> fn)
{
    m_error_callbackfn = fn;
}

void Connection::set_handle_message_callbackfn(std::function<void(spConnection, std::shared_ptr<std::string>)> fn)
{
    m_handle_message_callbackfn = fn;
}

void Connection::set_sendcomplete_callbackfn(std::function<void(spConnection)> fn)
{
    m_sendcomplete_callbackfn = fn;
}

