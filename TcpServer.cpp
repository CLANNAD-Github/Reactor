#include "TcpServer.h"
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>

TcpServer::TcpServer(const std::string& ip, uint16_t port, int io_thread, int timer, int timeout, enum data_type type)
    : m_mainloop(new EventLoop(true, timer, timeout)),
    m_acceptor(m_mainloop.get(), ip, port),
    m_threadpool(io_thread, "IO"),
    m_data_type(type)
{
    m_acceptor.set_handle_new_connection_callback_fn(std::bind(&TcpServer::HandleNewConnection, this, std::placeholders::_1));
    m_mainloop->set_timeout_callback_fn(std::bind(&TcpServer::HandleTimeout, this, std::placeholders::_1));

    for (int i = 0; i < io_thread; i++)
    {
        m_subloop.emplace_back(new EventLoop(false, timer, timeout));
        m_subloop[i]->set_timeout_callback_fn(std::bind(&TcpServer::HandleTimeout, this, std::placeholders::_1));
        m_subloop[i]->set_remove_connection_callback_fn(std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
    }
}
 
void TcpServer::start(int epoll_timeout)
{
    printf("main thread id %d.\n", syscall(SYS_gettid));
    for (int i = 0; i < m_threadpool.size(); i++)
        m_threadpool.add_task(std::bind(&EventLoop::run, m_subloop[i].get(), epoll_timeout));
    m_mainloop->run(epoll_timeout);
}

void TcpServer::stop()
{
    m_mainloop->stop();
    
    for (auto & e : m_subloop)
        e->stop();

    m_threadpool.stop();
}

TcpServer::~TcpServer()
{}

void TcpServer::HandleNewConnection(std::unique_ptr<Socket> client_socket)
{
    int sub = client_socket->fd() % m_subloop.size();
    spConnection new_connection(new Connection(m_subloop[sub].get(), std::move(client_socket), m_data_type));
    new_connection->set_handle_message_callback_fn(std::bind(&TcpServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    new_connection->set_handle_send_complete_callback_fn(std::bind(&TcpServer::HandleSendComplete, this, std::placeholders::_1));
    new_connection->set_handle_close_connection_callback_fn(std::bind(&TcpServer::HandleCloseConnection, this, std::placeholders::_1));
    new_connection->set_handle_error_connection_callback_fn(std::bind(&TcpServer::HandleErrorConnection, this, std::placeholders::_1));

    {
        std::lock_guard<std::mutex> lock(m_mutex_map);
        m_map_con[new_connection->fd()] = new_connection;
    }
    m_subloop[sub]->add_connection(new_connection);

    if (m_handle_new_connection_callback_fn)
        m_handle_new_connection_callback_fn(new_connection);
}

void TcpServer::HandleMessage(spConnection con, std::string& message)
{
    if (m_handle_message_callback_fn)
        m_handle_message_callback_fn(con, message);
}

void TcpServer::HandleSendComplete(spConnection con)
{
    if (m_handle_send_complete_callback_fn)
        m_handle_send_complete_callback_fn(con);
}

void TcpServer::HandleErrorConnection(spConnection con)
{
    if (m_handle_error_connection_callback_fn)
        m_handle_error_connection_callback_fn(con);
    remove_connection(con);
}

void TcpServer::HandleCloseConnection(spConnection con)
{
    if (m_handle_close_connection_callback_fn)
        m_handle_close_connection_callback_fn(con);
    remove_connection(con);
}

void TcpServer::HandleTimeout(EventLoop * eventloop)
{
    if (m_handle_timeout_callback_fn)
        m_handle_timeout_callback_fn();    
}

void TcpServer::remove_connection(spConnection con)
{
    std::lock_guard<std::mutex> lock(m_mutex_map);
    m_map_con.erase(con->fd());
}

void TcpServer::set_handle_new_connection_callback_fn(std::function<void (spConnection)> fn)        { m_handle_new_connection_callback_fn = fn; }
void TcpServer::set_handle_message_callback_fn(std::function<void (spConnection, std::string&)> fn)  { m_handle_message_callback_fn = fn; }
void TcpServer::set_handle_send_complete_callback_fn(std::function<void (spConnection)> fn)         { m_handle_send_complete_callback_fn = fn; }
void TcpServer::set_handle_error_connection_callback_fn(std::function<void (spConnection)> fn)      { m_handle_error_connection_callback_fn = fn; }
void TcpServer::set_handle_close_connection_callback_fn(std::function<void (spConnection)> fn)      { m_handle_close_connection_callback_fn = fn; }
void TcpServer::set_handle_timeout_callback_fn(std::function<void ()> fn)                           { m_handle_timeout_callback_fn = fn; }
