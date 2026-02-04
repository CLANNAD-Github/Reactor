#include "EchoServer.h"
#include "Connection.h"
#include <iostream>
#include <sys/syscall.h>

EchoServer::EchoServer(const std::string& ip, uint16_t port, int io_thread_num, int work_thread_num, int timer, int con_timeout, enum data_type type)
    : m_server(ip, port, io_thread_num, timer, con_timeout, type), m_work_thread_pool(work_thread_num, "WORK")
{
    m_server.set_handle_new_connection_callback_fn(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    m_server.set_handle_message_callback_fn(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_server.set_handle_send_complete_callback_fn(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    m_server.set_handle_close_connection_callback_fn(std::bind(&EchoServer::HandleCloseConnection, this, std::placeholders::_1));
    m_server.set_handle_error_connection_callback_fn(std::bind(&EchoServer::HandleErrorConnection, this, std::placeholders::_1));
    // m_server.set_handle_timeout_callback_fn(std::bind(&EchoServer::HandleTimeout, this));
}

EchoServer::~EchoServer()
{
    stop();
}

void EchoServer::start(int timeout)
{
    m_server.start(timeout);
}

void EchoServer::stop()
{
    m_work_thread_pool.stop();
    m_server.stop();
}

void EchoServer::HandleNewConnection(spConnection con)
{
    printf("new client(%s-%d) fd:%d connected. start time:%d\n", con->ip().c_str(), con->port(), con->fd(), TimeStamp::now());
}

// 接收到消息
void EchoServer::HandleMessage(spConnection con, std::string& message)
{
    // printf("Connection fd:%d in thread %d recv message:%s\n", con->fd(), syscall(SYS_gettid), message.c_str());

    std::shared_ptr<std::string> pmsg(new std::string(message));
    if (m_work_thread_pool.size() == 0)
    {
        message_response(con, std::move(pmsg));
    }
    else
    {
        m_work_thread_pool.add_task(std::bind(&EchoServer::message_response, this, con, std::move(pmsg)));
    }
}

void EchoServer::message_response(spConnection con, std::shared_ptr<std::string> message)
{
    // 计算消息
    std::string msg_res = *message;

    // 发送回应
    con->send_message(msg_res.data(), msg_res.size());
}

void EchoServer::HandleSendComplete(spConnection con)
{
    // printf("Connetion fd(%d) message send complete.\n", con->fd());
}

void EchoServer::HandleErrorConnection(spConnection con)
{
    printf("Connection error, fd:%d end time:%d\n", con->fd(), TimeStamp::now());
}

void EchoServer::HandleCloseConnection(spConnection con)
{
    printf("Connection close, fd:%d end time:%d\n", con->fd(), TimeStamp::now());
}

// void EchoServer::HandleTimeout()
// {
//     printf("EchoServer timeout.\n");
// }

