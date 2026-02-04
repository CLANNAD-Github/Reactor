#pragma once

#include <sys/epoll.h>
#include <vector>

class Channel;

class Epoll
{
private:
    static const int MaxEvent = 128;
    const int m_epollfd;
    epoll_event m_arr_ev[MaxEvent];
public:
    Epoll();
    ~Epoll();

    bool update_channel(Channel* ch);
    bool remove_channel(Channel* ch);
    std::vector<Channel*> loop(int timeout);
};
