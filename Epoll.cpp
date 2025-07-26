#include "Epoll.h"
#include "Channel.h"
#include <unistd.h>

Epoll::Epoll()
{
    m_epollfd = epoll_create(1);
    if (m_epollfd == -1)
    {
        perror("epoll_create() faild.\n");
        exit(-1);
    }
}

Epoll::~Epoll()
{
    printf("~Epoll() m_epollfd = %d\n", m_epollfd);
    close(m_epollfd);
}

void Epoll::add_channel(Channel * ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->get_event();
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, ch->get_fd(), &ev);   
}

void Epoll::update_channel(Channel * ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->get_event();
    if (ch->in_epoll() == true)
    {
        if(epoll_ctl(m_epollfd, EPOLL_CTL_MOD, ch->get_fd(), &ev)== -1)
        {
            perror("epoll_ctl() MOD faild.\n");
            exit(-1);
        }
    }
    else
    {
        if(epoll_ctl(m_epollfd, EPOLL_CTL_ADD, ch->get_fd(), &ev) == -1)
        {
            perror("epoll_ctl() MOD faild.\n");
            exit(-1);
        }
        ch->set_inepoll();
    }
}

void Epoll::remove_channel(Channel * ch)
{
    if (ch->in_epoll())
    {
        printf("Epoll::remove_channel, ch->fd(%d)\n", ch->get_fd());
        if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, ch->get_fd(), NULL) == -1)
        {
            printf("epoll_ctl() DEL faild. exit.\n");
            exit(-1);
        }
    }
}

std::vector<Channel*> Epoll::loop(int timeout)
{
    // epoll_wait 函数会被阻塞，直到有消息进来，注意该阻塞在那个线程中
    int res = epoll_wait(m_epollfd, m_evs, MaxEvents, timeout);
        
    if (res < 0)
    {
        perror("epoll_wait() faild.\n");
        exit(-1);
    }

    std::vector<Channel *>vec_ch;
    Channel * ch;
    for (int i = 0; i < res; i++)
    {
        ch = (Channel*)m_evs[i].data.ptr;
        ch->set_revent(m_evs[i].events);
        vec_ch.push_back(ch);
    }
    return vec_ch;
}

