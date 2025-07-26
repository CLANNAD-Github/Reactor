
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include "InetAddress.h"

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("Using: program serverip serverport\n");
        printf("Example:./tcpclient 192.168.1.101 2001\n");
        exit(-1);
    }

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        printf("初始化 socket faild. client_fd = %d\n", client_fd);
        return -1;
    }

    int opt = 1;
    setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    InetAddress server_addr(argv[1], atoi(argv[2]));
    
    if (connect(client_fd, server_addr.addr(), sizeof(sockaddr)) != 0)
    {

        printf("client connect %s:%s faild\n", argv[1], argv[2]);
        close(client_fd);
        exit(-1);
    }
    printf("client connect %s:%s ok\n", argv[1], argv[2]);

    char buffer[1024];

    /*
    while (true)
    {
        memset(buffer, '\0', sizeof(buffer));
        printf("please input message:");
        fgets(buffer, 1024, stdin);

        if (strcmp(buffer, "quit\n") == 0)
        {
            printf("程序退出.\n");
            break;
        }

        if(send(client_fd, buffer, strlen(buffer), 0) <= 0)
        {
            printf("send() faild. client disconnected.\n");
            break;
        }

        memset(buffer, '\0', sizeof(buffer));
        if (recv(client_fd, buffer, sizeof(buffer), 0) <= 0)
        {
            printf("recv() faild. client disconnected.\n");
            break;
        }
        else
        {
            printf("recv message:%s", buffer);
        }
    }
    */

    //////////////////////////////
    // TCP 沾包和分包的情景测试
    //////////////////////////////
    // 客户端快速发送一批数据
    for (int i = 0; i < 1; i++)
    {
        // 为了解决粘包分包的问题，可以在发送的数据前添加 4 字节的整数，表示该条数据的长度
        char temp[1024];
        memset(temp, '\0', sizeof(temp));
        memset(buffer, '\0', sizeof(buffer));
        int len = sprintf(temp, "This is %d message.", i);
        memcpy(buffer, &len, sizeof(int));
        memcpy(buffer + sizeof(int), temp, len);
        send(client_fd, buffer, len + sizeof(int), 0);
    }

    // 发送之后在接受回应，这就会造成粘包和分包的情况发生
    for (int i = 0; i < 1; i++)    
    {
        memset(buffer, '\0', sizeof(buffer));
        int len = 0;
        recv(client_fd, &len, sizeof(int), 0);
        recv(client_fd, buffer, len, 0);
        printf("recv:%s\n", buffer);
    }  

    // sleep(30);
    close(client_fd);
    printf("client_fd closed.\n");

    return 0;
}

