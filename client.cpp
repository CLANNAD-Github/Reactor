#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <iostream>
#include "InetAddress.h"
#include <time.h>

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage:./client ip port num\n");
        fprintf(stderr, "Example:./client 192.168.1.102 8050 10000\n");
        exit(EXIT_FAILURE);
    }

    int client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_fd == -1)
    {
        fprintf(stderr, "%s %d error:%d socket error.", __FILE__, __LINE__, errno);
        exit(EXIT_FAILURE);
    }

    InetAddress server_addr(argv[1], atoi(argv[2]));

    if(connect(client_fd, server_addr.addr(), (socklen_t)sizeof(struct sockaddr_in)) != 0)
    {
        fprintf(stderr, "connect to server(%s:%d) faild.\n", argv[1], atoi(argv[2]));
        exit(EXIT_FAILURE);
    }

    // fprintf(stdout, "connect to server(%s:%d) ok.\n", argv[1], atoi(argv[2]));

    int num = atoi(argv[3]);
    int len = 0;
    char buffer[64];

    printf("fd %d start time:%d.\n", client_fd, time(NULL));

    for (size_t i = 0; i < num; i++)
    {
        memset(buffer, '\0', sizeof(buffer));
        len = sprintf(buffer + 4, "This is %d message.", i);
        memcpy(buffer, (char*)&len, 4);

        int res = send(client_fd, buffer, len+4, 0);

        if (res <= 0)
        {
            fprintf(stderr, "send data faild.\n");
        }
        // printf("send %d data %s\n", len, buffer + 4);
    // }
    // printf("fd %d send complete time:%d.\n", client_fd, time(NULL));

    // for (size_t i = 0; i < num; i++)
    // {
        recv(client_fd, &len, sizeof(int), 0);
        // printf("recv %d bytes data. ", len);

        memset(buffer, '\0', sizeof(buffer));
        recv(client_fd, buffer, len, 0);
        // printf("data:%s\n", buffer);
        
        // sleep(1);
    }

    printf("fd %d End time:%d.\n", client_fd, time(NULL));
    close(client_fd);
    return 0;
}
