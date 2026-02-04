#pragma once

#include "TimeStamp.h"
#include "Buffer.h"
#include <functional>
#include <memory>
#include <atomic>

class Socket;
class Channel;
class EventLoop;

class Connection : public std::enable_shared_from_this<Connection>
{
    using spConnection = std::shared_ptr<Connection>;
private:
    EventLoop * m_eventloop;
    std::unique_ptr<Socket> m_client_socket;
    std::unique_ptr<Channel> m_client_channel;
    
    std::atomic<bool> m_disconnect;

    Buffer m_recv_buffer;
    Buffer m_send_buffer;

    TimeStamp m_lasttime;

    std::function<void (spConnection, std::string&) > m_handle_message_callback_fn;
    std::function<void (spConnection)> m_handle_send_complete_callback_fn;
    std::function<void (spConnection)> m_handle_close_connection_callback_fn;
    std::function<void (spConnection)> m_handle_error_connection_callback_fn;

public:
    Connection(EventLoop * eventloop, std::unique_ptr<Socket> client_socket, enum data_type type);
    ~Connection();

    std::string ip() const;
    uint16_t port() const;
    int fd() const;

    TimeStamp time_stamp() const;
    bool disconnect() const;
    void remove_from_eventloop();

    bool in_io_thread() const;

    void recv_callback();
    void send_callback();
    void error_callback();
    void close_callback();

    void send_message(const char* data, size_t size);
    void send_message_in_loop(std::shared_ptr<std::string> message);

    void set_handle_message_callback_fn(std::function<void (spConnection, std::string&)> fn);
    void set_handle_send_complete_callback_fn(std::function<void (spConnection)> fn);
    void set_handle_close_connection_callback_fn(std::function<void (spConnection)> fn);
    void set_handle_error_connection_callback_fn(std::function<void (spConnection)> fn);
};
