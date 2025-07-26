#include "TcpServer.h"
#include <string.h>

TcpServer::TcpServer(const std::string & ip, const uint32_t port, int thread_num)
    // 指定时间循环是否为主事件循环
    : m_evloop(new EventLoop(new Epoll(), true)), m_thread_num(thread_num), m_acceptor(m_evloop.get(), ip, port), m_tp(new ThreadPool(m_thread_num, "IO"))
{
    m_acceptor.set_handle_connection_callbackfn(std::bind(&TcpServer::handle_connection, this, std::placeholders::_1));
    // 在构造函数中初始化evloop类中的回调函数
    m_evloop->set_epolltimeout_callbackfn(std::bind(&TcpServer::epoll_timeout, this));
    
    for (int i = 0; i < m_thread_num; i++)
    {
        // 创建临时指针变量，然后移动给容器中的 指针变量，这里创建的都是从事循环
        std::unique_ptr<EventLoop> temp_up(new EventLoop(new Epoll(), false));
        m_subevloops.push_back(std::move(temp_up));
        m_subevloops[i]->set_epolltimeout_callbackfn(std::bind(&TcpServer::epoll_timeout, this));
        m_subevloops[i]->set_erase_connection_callbackfn(std::bind(&TcpServer::erase_connection, this, std::placeholders::_1));
        // 添加任务到线程，立即开始运行
        m_tp->add_task(std::bind(&EventLoop::loop, m_subevloops[i].get()));
    }
}

TcpServer::~TcpServer()
{
    m_subevloops.clear();
    m_map_con.clear();
}

void TcpServer::start()
{
    m_evloop->loop();
}

void TcpServer::stop()
{
    // 终止主事件循环
    m_evloop->stop();
    // 终止子事件循环
    for (const auto &loop : m_subevloops)
    {
        loop->stop();
    }
    // 终止IO线程，工作线程在 echoserver（上层类） 中释放
    m_tp->stop();
}

void TcpServer::handle_connection(std::shared_ptr<Socket> client_socket)
{
    // 在主线程中接受客户端链接，并将客户端链接分配到子线程中
    // 新的链接在主线程中分配，在子线程中运行
    spConnection client_connection (new Connection(m_subevloops[client_socket->get_fd()%m_subevloops.size()].get(), client_socket));
    client_connection->set_sendcomplete_callbackfn(std::bind(&TcpServer::send_complete, this, std::placeholders::_1));
    client_connection->set_close_callbackfn(std::bind(&TcpServer::close_connection, this, std::placeholders::_1));
    client_connection->set_error_callbackfn(std::bind(&TcpServer::error_connection, this, std::placeholders::_1));
    client_connection->set_handle_message_callbackfn(std::bind(&TcpServer::handle_message, this, std::placeholders::_1, std::placeholders::_2));
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map_con[client_connection->get_fd()] = client_connection;
    }

    // 在 EventLoop 对象中还需要添加一次 Connection 对象
    m_subevloops[client_connection->get_fd() % m_subevloops.size()]->add_Connection(client_connection);

    if (m_handle_connection_callback != nullptr)
        m_handle_connection_callback(client_socket);
}

void TcpServer::close_connection(spConnection con)
{
    if (m_close_connection_callback != nullptr)
        m_close_connection_callback(con);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map_con.erase(con->get_fd());
    }
}

void TcpServer::error_connection(spConnection con)
{
    if (m_error_connection_callback != nullptr)
        m_error_connection_callback(con);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map_con.erase(con->get_fd());
    }
}

void TcpServer::handle_message(spConnection con, std::shared_ptr<std::string> message)
{
    if (m_handle_message_callback != nullptr)
        m_handle_message_callback(con, message);
}

void TcpServer::send_complete(spConnection con)
{
    if (m_send_complete_callback != nullptr)
        m_send_complete_callback(con);
}

void TcpServer::epoll_timeout()
{
    if (m_epoll_timeout_callback != nullptr)
        m_epoll_timeout_callback();
}

// 根据 fd 删除 Connection 对象。供 EventLoop 对象回调
// 注意，因为 子循环中才会调用该函数，因此，该函数会运行在 子线程中，这和前面的 map 相关操作不在同一个线程中，因此这里需要加锁
void TcpServer::erase_connection(int fd)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map_con.erase(fd);
    }
}

void TcpServer::set_handle_connection_callback(std::function<void (std::shared_ptr<Socket>)> fn)
{
    m_handle_connection_callback = fn;
}

void TcpServer::set_close_connection_callback(std::function<void (spConnection)> fn)
{
    m_close_connection_callback = fn;
}

void TcpServer::set_error_connection_callback(std::function<void (spConnection)> fn)
{
    m_error_connection_callback = fn;
}

void TcpServer::set_handle_message_callback(std::function<void (spConnection, std::shared_ptr<std::string>)> fn)
{
    m_handle_message_callback = fn;
}

void TcpServer::set_send_complete_callback(std::function<void (spConnection)> fn)
{
    m_send_complete_callback = fn;
}

void TcpServer::set_epoll_timeout_callback(std::function<void ()> fn)
{
    m_epoll_timeout_callback = fn;
}

