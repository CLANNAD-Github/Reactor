#pragma once

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "Timestamp.h"
#include <functional>
#include <atomic>

class Connection;
using spConnection = std::shared_ptr<Connection>;

class Connection : public std::enable_shared_from_this<Connection>
{
private:
    // 注意成员初始化顺序
    EventLoop * m_evloop;
    std::shared_ptr<Socket> m_client_socket;
    std::shared_ptr<Channel> m_client_channal;
    std::atomic_bool m_disconnect; // 用于表示是否链接的状态，在工作线程和IO线程中都会用到，因此使用原子类型数据避免竞争
    Timestamp m_tsp;
    
    Buffer m_inputbuffer;
    Buffer m_outputbuffer;

    std::function<void(spConnection)> m_sendcomplete_callbackfn;
    std::function<void(spConnection, std::shared_ptr<std::string>)> m_handle_message_callbackfn;
    std::function<void(spConnection)> m_close_callbackfn;
    std::function<void(spConnection)> m_error_callbackfn;
public:
    Connection(EventLoop* evloop, std::shared_ptr<Socket> client_socket);
    ~Connection();

    std::string get_ip() const;
    uint16_t get_port() const;
    int get_fd() const;
    bool istimeout(time_t t, int val) const; // 判断该Connection 是否超时, t 为指定的时间戳，val为超时的时间（单位：秒）

    void handle_message();
    void send(std::shared_ptr<std::string> data);
    void send_in_io_loop(std::shared_ptr<std::string> data); // 在事件循环中执行，将该函数放在事件循环的任务队列中

    void remove_channel();
    void write_callback();
    void close_callback();
    void error_callback();

    void set_sendcomplete_callbackfn(std::function<void(spConnection)> fn);
    void set_close_callbackfn(std::function<void(spConnection)> fn);
    void set_error_callbackfn(std::function<void(spConnection)> fn);
    void set_handle_message_callbackfn(std::function<void(spConnection, std::shared_ptr<std::string>)> fn);
};
