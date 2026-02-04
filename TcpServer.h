#include <stdint.h>
#include <string>
#include "Acceptor.h"
#include <map>
#include "ThreadPool.h"
#include <memory>
#include "Connection.h"

class Connection;

using spConnection = std::shared_ptr<Connection>;

class TcpServer
{
private:
    std::unique_ptr<EventLoop> m_mainloop;
    std::vector<std::unique_ptr<EventLoop>> m_subloop;
    ThreadPool m_threadpool;    
    Acceptor m_acceptor;
    
    std::mutex m_mutex_map;
    std::map<int, spConnection> m_map_con;
    
    enum data_type m_data_type;

    std::function<void (spConnection)> m_handle_new_connection_callback_fn;
    std::function<void (spConnection, std::string&)> m_handle_message_callback_fn;
    std::function<void (spConnection)> m_handle_send_complete_callback_fn;
    std::function<void (spConnection)> m_handle_error_connection_callback_fn;
    std::function<void (spConnection)> m_handle_close_connection_callback_fn;
    std::function<void ()> m_handle_timeout_callback_fn;

    void remove_connection(spConnection con);

public:
    TcpServer(const std::string& ip, uint16_t port, int io_thread = 1, int timer = -1, int timeout = -1, enum data_type type = data_type::T_INT_HEAD);
    ~TcpServer();

    void start(int epoll_timeout = -1);
    void stop();

    void HandleNewConnection(std::unique_ptr<Socket> client_socket);
    void HandleMessage(spConnection con, std::string& message);
    void HandleSendComplete(spConnection con);
    void HandleErrorConnection(spConnection con);
    void HandleCloseConnection(spConnection con);
    void HandleTimeout(EventLoop * eventloop);

    void set_handle_new_connection_callback_fn(std::function<void (spConnection)> fn);
    void set_handle_message_callback_fn(std::function<void (spConnection, std::string&)> fn);
    void set_handle_send_complete_callback_fn(std::function<void (spConnection)> fn);
    void set_handle_error_connection_callback_fn(std::function<void (spConnection)> fn);
    void set_handle_close_connection_callback_fn(std::function<void (spConnection)> fn);
    void set_handle_timeout_callback_fn(std::function<void ()> fn);
};
