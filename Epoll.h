#pragma once
#include <sys/epoll.h>
#include <vector>

class Channel;

class Epoll
{
private:
    static const int MaxEvents = 128;
    int m_epollfd;
    epoll_event m_evs[MaxEvents];
    int m_timeout;
public:
    Epoll(int timeout);
    ~Epoll();

    void add_channel(Channel * ch);
    void update_channel(Channel * ch);
    void remove_channel(Channel * ch);
    std::vector<Channel*> loop();
};
