#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <sys/syscall.h>

Connection::Connection(EventLoop * eventloop, std::unique_ptr<Socket> client_socket, enum data_type type)
    :
    m_eventloop(eventloop),
    m_client_socket(std::move(client_socket)),
    m_client_channel(new Channel(m_eventloop, m_client_socket->fd())),
    m_recv_buffer(type),
    m_send_buffer(type),
    m_lasttime(),
    m_disconnect(false)
{
    m_client_channel->enable_read();
    // m_client_channel->enable_et();
    m_client_channel->set_read_callback_fn(std::bind(&Connection::recv_callback, this));
    m_client_channel->set_write_callback_fn(std::bind(&Connection::send_callback, this));
    m_client_channel->set_close_callback_fn(std::bind(&Connection::close_callback, this));
    m_client_channel->set_error_callback_fn(std::bind(&Connection::error_callback, this));

    m_eventloop->update_channel(m_client_channel.get());
}

Connection::~Connection()
{
    // printf("~Connection() fd: %d\n", fd());
}

std::string Connection::ip() const { return m_client_socket->ip(); }
uint16_t Connection::port() const { return m_client_socket->port(); }
int Connection::fd() const { return m_client_socket->fd(); }
bool Connection::disconnect() const { return m_disconnect; }

// 因为 m_eventloop 成员是事件循环对象，该对象必然运行在 IO 线程池中，因此这里获取到的 thread_id 必然是 IO 线程ID
// 然后与syscall函数返回的线程ID进行比对就可以判断出该函数调用发生在 IO 线程还是在 work 线程
bool Connection::in_io_thread() const { return m_eventloop->thread_id() == syscall(SYS_gettid); }
TimeStamp Connection::time_stamp() const { return m_lasttime; }
void Connection::remove_from_eventloop() { m_eventloop->remove_connection(shared_from_this()); }

void Connection::recv_callback()
{
    m_lasttime.update();

    while (true)
    {
        int res = m_recv_buffer.recv_from_fd(fd());
        if (res > 0)
        {           
            std::string message;
            while (true)
            {
                if (m_recv_buffer.pick_message(message))
                    m_handle_message_callback_fn(shared_from_this(), message);
                else
                    break;
            }
        }
        else if (res == 0)
        {
            close_callback();
            break;
        }
        else if (res == -1 && (errno == EINTR))
        {
            fprintf(stderr, "client fd %d recv() interrupted.\n", fd());
        }
        else if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            std::string message;
            while (true)
            {
                if (m_recv_buffer.pick_message(message))
                    m_handle_message_callback_fn(shared_from_this(), message);
                else
                    break;
            }
            break;
        }
        else
        {
            error_callback();
            break;
        }
    }
}

// 注意下面两个函数，send_message 可能运行在 IO 线程，也可能运行在 work 线程
void Connection::send_message(const char* data, size_t size)
{
    std::shared_ptr<std::string> msg(new std::string(data, size));
    
    if (in_io_thread())
    {
        send_message_in_loop(std::move(msg));
    }
    else
    {
        m_eventloop->push_queue(std::bind(&Connection::send_message_in_loop, this, std::move(msg)));
    }
}

void Connection::send_message_in_loop(std::shared_ptr<std::string> message)
{
    m_send_buffer.append(message->data(), message->size());
    m_client_channel->enable_write();
}

void Connection::send_callback()
{
    m_lasttime.update();

    if (m_send_buffer.empty())
    {
        m_client_channel->disable_write();
        m_handle_send_complete_callback_fn(shared_from_this());
    }
    else
    {
        if (!disconnect())
        {
            m_send_buffer.send_to_fd(fd());
        }
    }
}

void Connection::error_callback()
{
    m_disconnect = true;
    m_eventloop->remove_channel(m_client_channel.get());
    m_handle_error_connection_callback_fn(shared_from_this());
    // close(fd());
    m_eventloop->remove_connection(shared_from_this());
}

void Connection::close_callback()
{
    m_disconnect = true;
    m_eventloop->remove_channel(m_client_channel.get());
    m_handle_close_connection_callback_fn(shared_from_this());
    // close(fd());
    m_eventloop->remove_connection(shared_from_this());
}


void Connection::set_handle_message_callback_fn(std::function<void (spConnection, std::string&)> fn) { m_handle_message_callback_fn = fn; }
void Connection::set_handle_send_complete_callback_fn(std::function<void (spConnection)> fn) { m_handle_send_complete_callback_fn = fn; }
void Connection::set_handle_close_connection_callback_fn(std::function<void (spConnection)> fn) { m_handle_close_connection_callback_fn = fn; }
void Connection::set_handle_error_connection_callback_fn(std::function<void (spConnection)> fn) { m_handle_error_connection_callback_fn = fn; }
