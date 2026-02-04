#include "EchoServer.h"
#include <iostream>
#include <signal.h>

void EXIT(int sig);

EchoServer * echoserver;

int main(int argc, char const *argv[])
{
    if (argc != 8)
    {
        printf("Usage:./echoserver ip port io_thread work_thread timer con_timeout data_type\n");
        printf("Example:./echoserver 192.168.1.102 8050 3 3 10 10 1\n");
        printf("ip:服务器IP地址\n");
        printf("port:服务器port\n");
        printf("io_thread:服务器 IO 线程数量，用于处理客户端连接与数据收发工作\n");
        printf("work_thread:服务器 work 线程数量，用于处理客户端报文，适合处理报文较为耗时的情况\n");
        printf("timer:服务器定时器间隔，单位（S）\n");
        printf("con_timeout:客户端连接超时时间，单位（S）\n");
        printf("data_type:通信数据的格式类型，0 表示无类型 1 表示 4 字节整数开头\n");

        exit(EXIT_FAILURE);
    }

    signal(SIGTERM, EXIT);
    signal(SIGINT, EXIT);

    echoserver = new EchoServer(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), (enum data_type)atoi(argv[7]));
    echoserver->start(5000);
    
    return 0;
}

void EXIT(int sig)
{
    printf("echoserver EXIT. sig = %d\n", sig);
    delete echoserver;
}
