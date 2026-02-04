
#pragma once

#include "TcpServer.h"
#include <string>

class EchoServer
{
private:
    TcpServer m_server;
    ThreadPool m_work_thread_pool;
public:
    EchoServer(const std::string& ip, uint16_t port, int io_thread_num, int work_thread_num, int timer = -1, int con_timeout = -1, enum data_type type = data_type::T_INT_HEAD);
    ~EchoServer();

    void start(int epolltimeout = -1);
    void stop();
    
    void HandleNewConnection(spConnection con);
    void HandleMessage(spConnection con, std::string&);
    void HandleSendComplete(spConnection con);
    void HandleErrorConnection(spConnection con);
    void HandleCloseConnection(spConnection con);
    // void HandleTimeout();

    void message_response(spConnection con, std::shared_ptr<std::string> message);
};
