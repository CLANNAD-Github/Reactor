#include "Acceptor.h"

Acceptor::Acceptor(EventLoop * eventloop, const std::string& ip, uint16_t port) :
    m_eventloop(eventloop),
    m_server_socket(new Socket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), ip, port)),
    m_server_channel(new Channel(m_eventloop, m_server_socket->fd(), true))
{
    m_server_socket->setkeepalive();
    m_server_socket->setnodelay();
    m_server_socket->setreuseaddr();
    m_server_socket->setreuseport();
    
    InetAddress server_addr(ip, port);
    m_server_socket->bind(server_addr);
    m_server_socket->listen();
    
    m_server_channel->set_read_callback_fn(std::bind(&Acceptor::handle_new_connection, this));
    m_server_channel->enable_read();
}

Acceptor::~Acceptor()
{}

int Acceptor::fd() const { return m_server_socket->fd(); }
EventLoop * Acceptor::eventloop() const { return m_eventloop; }

void Acceptor::handle_new_connection()
{
    InetAddress client_addr;
    int client_fd = m_server_socket->accept(client_addr);

    std::unique_ptr<Socket> client_socket(new Socket(client_fd, client_addr.ip(), client_addr.port()));
    client_socket->setnonblock();
    m_handle_new_connection_callback_fn(std::move(client_socket));
}

void Acceptor::set_handle_new_connection_callback_fn(std::function<void (std::unique_ptr<Socket>)> fn) { m_handle_new_connection_callback_fn = fn; }
