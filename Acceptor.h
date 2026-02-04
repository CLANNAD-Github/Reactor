#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include "Epoll.h" 
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include <memory>

class Acceptor
{
private:
    EventLoop* m_eventloop;
    std::unique_ptr<Socket> m_server_socket;
    std::unique_ptr<Channel> m_server_channel;
    std::function<void (std::unique_ptr<Socket>)> m_handle_new_connection_callback_fn;
public:
    Acceptor(EventLoop * eventloop, const std::string& ip, uint16_t port);
    ~Acceptor();

    int fd() const;
    EventLoop * eventloop() const;

    void handle_new_connection();
    void set_handle_new_connection_callback_fn(std::function<void (std::unique_ptr<Socket>)> fn);

};
