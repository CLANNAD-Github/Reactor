
#include "EchoServer.h"
#include <signal.h>

EchoServer * echoserver;

void EXIT(int sig);

int main(int argc, char const *argv[])
{
    if (argc != 7)
    {
        printf("Using:./echoserver ip port IOThreadNum workthreadnum timeout con_timeout\n");
        printf("Example:./echoserver 192.168.1.102 2001 3 3 10000 60\n");

        printf("ip:服务端ip\n");
        printf("port:服务端port\n");
        printf("IOThreadNum:服务端IO线程数量\n");
        printf("workThreadNum:服务端工作线程数量\n");
        printf("timeout:epoll超时时间\n");
        printf("con_timeout:客户端链接超时时间\n");

        exit(-1);
    }
    
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    echoserver = new EchoServer(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), 60);
    echoserver->start();

    return 0;
}

void EXIT(int sig)
{
    printf("Server Exit. sig = %d\n", sig);
    delete echoserver;
    exit(0);
}