
#include "EchoServer.h"
#include <signal.h>

EchoServer * echoserver;

void EXIT(int sig);

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("Using:./echoserver ip port\n");
        printf("Example:./echoserver 192.168.1.101 2001\n");
        exit(-1);
    }
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    echoserver = new EchoServer(argv[1], atoi(argv[2]), 3, 3);
    echoserver->start();

    return 0;
}

void EXIT(int sig)
{
    printf("Server Exit. sig = %d\n", sig);
    delete echoserver;
    exit(0);
}