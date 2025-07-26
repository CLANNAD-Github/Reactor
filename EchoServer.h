
#include "TcpServer.h"
#include "ThreadPool.h"
#include <memory>

/*EchoServer 是具体的业务类，进行回显数据，其他的业务类可以根据该模板任意定制*/
// 在 EchoServer 中指定工作线程的数量，让数据的处理工作由工作线程处理，IO线程处理IO请求
// 因为 IO 处理过程由 TcpServer 包办，因此只需要在构造 TcpServer 对象时指定IO线程数量即可
// 但是数据的处理过程是由业务类决定的，因此业务类应添加工作线程对象，以及处理数据的相关函数，然后在数据到达需要处理时指定线程处理

class EchoServer
{
private:
    TcpServer m_server;
    std::unique_ptr<ThreadPool> m_work_tp;
public:
    EchoServer(const std::string & ip, const uint32_t port, int thread_num_io = 3, int thread_num_work = 3);
    ~EchoServer();

    // 从 TcpServer 提供的接口中选择出来进行具体实现    
    void start();
    void stop();
    void handle_connection(std::shared_ptr<Socket> client_socket);
    void close_connection(spConnection con);
    void error_connection(spConnection con);
    void handle_message(spConnection con, std::shared_ptr<std::string>); // 服务端处理消息的函数（回调 Connection 类中的函数）
    void send_complete(spConnection con);
    void epoll_timeout();

    // 处理传递来的消息，上面的 handle_message 函数用于接受消息，但不进行业务上的处理，且handle_message函数工作在 IO 线程中
    // 本函数在工作线程中运行，从而避免 IO 线程阻塞在处理数据过程中
    void on_message(spConnection con, std::shared_ptr<std::string>);
};
