#include "Acceptor.h"

Acceptor::Acceptor(EventLoop* evloop, const std::string & ip, const uint32_t port)
    : m_evloop(evloop), m_server_socket(create_nonblock_listenfd()), m_accept_channal(m_evloop, m_server_socket.get_fd())
{
    InetAddress server_addr(ip, port);
    m_server_socket.set_keepalive(true);
    m_server_socket.set_reuseaddr(true);
    m_server_socket.set_reuseport(true);
    m_server_socket.set_tcpnodelay(true);
    m_server_socket.bind(server_addr);
    m_server_socket.listen();

    m_accept_channal.enable_read();
    m_accept_channal.set_read_callbackfn(std::bind(&Acceptor::handle_connection, this));
}

Acceptor::~Acceptor()
{}

void Acceptor::handle_connection()
{
    InetAddress client_addr;
    int client_fd = m_server_socket.accept(client_addr);
    // 这里创建的 Socket 是属于 Conenction 的Socket，下面调用的回调函数，将最终传给 Connection 所有，因此这里采用 unique_ptr 传递就可，使用移动语义
    std::shared_ptr<Socket> client_socket (new Socket(client_fd, client_addr.get_ip(), client_addr.get_port()));
    // 回调 TcpServer 类中的函数，注意使用移动语义
    m_handle_connection_callbackfn(std::move(client_socket));
}

void Acceptor::set_handle_connection_callbackfn(std::function<void(std::shared_ptr<Socket>)> fn)
{
    m_handle_connection_callbackfn = fn;
}
