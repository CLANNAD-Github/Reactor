#include "EchoServer.h"
#include <functional>
#include <iostream>
#include "Timestamp.h"

EchoServer::EchoServer(const std::string & ip, const uint32_t port, int thread_num_io, int thread_num_work, int timeout, int con_timeout)
    :m_server(ip, port, thread_num_io, timeout), m_work_tp(new ThreadPool(thread_num_work, "WORK"))
{
    m_server.set_close_connection_callback(std::bind(&EchoServer::close_connection, this, std::placeholders::_1));
    m_server.set_epoll_timeout_callback(std::bind(&EchoServer::epoll_timeout, this));
    m_server.set_error_connection_callback(std::bind(&EchoServer::error_connection, this, std::placeholders::_1));
    m_server.set_handle_connection_callback(std::bind(&EchoServer::handle_connection, this, std::placeholders::_1));
    m_server.set_handle_message_callback(std::bind(&EchoServer::handle_message, this, std::placeholders::_1, std::placeholders::_2));
    m_server.set_send_complete_callback(std::bind(&EchoServer::send_complete, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{
    stop();
}

void EchoServer::start()
{
    m_server.start();
}

void EchoServer::stop()
{
    m_server.stop();
    m_work_tp->stop();
}

void EchoServer::handle_connection(std::shared_ptr<Socket> client_socket)
{
    printf("New Connection (fd:%d ip:%s port:%d) connected... handle_connection in Thread(%d) Start time(%s)\n",\
        client_socket->get_fd(), client_socket->get_ip().c_str(), client_socket->get_port(), syscall(SYS_gettid), Timestamp::now().tostring().c_str());
}

void EchoServer::close_connection(spConnection con)
{
    printf("Connection Closed, close_connection in Thread(%d) client_fd %d disconnect, close fd End time(%s).\n",\
        syscall(SYS_gettid), con->get_fd(), Timestamp::now().tostring().c_str());
}

void EchoServer::error_connection(spConnection con)
{
    printf("Connection Error, error_connection in Thread(%d) client_fd %d disconnect, close fd End time(%s).\n",\
        syscall(SYS_gettid), con->get_fd(), Timestamp::now().tostring().c_str());
}

void EchoServer::handle_message(spConnection con, std::shared_ptr<std::string> message)
{
    // 在工作线程中收到信息，将处理信息的任务交由工作线程处理
    // printf("EchoServer handle message in Thread(%d)\n", syscall(SYS_gettid));
   
    if (m_work_tp->size() == 0)
    {
        // 如果没有工作线程，则直接调用处理数据的函数，在IO线程中处理数据，这种方式适合处理数据的过程不费时间的情况
        on_message(con, message);
    }
    else
    {
        // 如果有工作线程，说明处理数据的过程比较耗时，为了不影响 IO 线程接收数据，需要将处理数据的过程放在 工作线程中
        // 使用 bind 函数绑定参数，使其变为 void () 类型函数，并在 工作线程 中运行
        
        // 注意这里新创建了一个智能指针及其string对象，是因为下面的函数将要放在 工作线程中执行，而此时仍然在IO线程中
        // 因此为了避免两个线程中的指针指向同一对象，需要再深拷贝一个智能指针传给工作线程
        std::shared_ptr<std::string> msg_send(new std::string(*message));
        m_work_tp->add_task(std::bind(&EchoServer::on_message, this, con, msg_send));
        // printf("EchoServer::handle_message() message.use_count = %d\n", message.use_count());
    }
}

void EchoServer::on_message(spConnection con, std::shared_ptr<std::string> message)
{
    // printf("on_message in Thread(%d)\n", syscall(SYS_gettid));
    // printf("EchoServer::on_message() message.use_count = %d\n", message.use_count());
    // 处理发送来的数据，根据业务需求决定
    // printf("recv:%s\n", message->c_str());

    // 因为下面的 send 函数有可能会被交由其他线程处理，因此，这里需要使用只能指针创建 buffer 对象，避免在另一线程中执行时，该函数已经退出， buffer 对象被析构
    std::shared_ptr<std::string> buffer(new std::string());
    *buffer = *message + " response.";

    // printf("send:%s\n", buffer->c_str());
    con->send(std::move(buffer));
}

void EchoServer::send_complete(spConnection con)
{
    // 注意发送完成不是每一个处理后的报文被发送就会调用该函数，而是发送一批数据后被调用（接收到的报文和发送的报文次数是不一定对应的）
    // Tcp连接是面向字节流，只负责发送字节流数据，send() 函数仅仅是把报文放在FD对应的发送缓冲区中，至于什么时候发送，完全由内核自行决定
    // 这也是粘包分包的原因
    
    // printf("EchoServer socket fd:(%d) send message ok. send_complete in Thread(%d)\n", con->get_fd(), syscall(SYS_gettid));
}

void EchoServer::epoll_timeout()
{
    // printf("EchoServer Timeout, in Thread(%d)\n", syscall(SYS_gettid));
}

