#include "Epoll.h"
#include "Channel.h"

#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

Epoll::Epoll() : m_epollfd(epoll_create(1))
{}

Epoll::~Epoll()
{
    close(m_epollfd);
}

std::vector<Channel*> Epoll::loop(int timeout)
{
    int res = epoll_wait(m_epollfd, m_arr_ev, MaxEvent, timeout);

    if (res < 0)
    {
        if(errno == EINTR)
            printf("epoll_wait() EINTR.\n");
        else
            fprintf(stderr, "%s %d %s\n", __FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    std::vector<Channel*> vec_ch;
    for (size_t i = 0; i < res; i++)
    {
        Channel* ch = (Channel*)m_arr_ev[i].data.ptr;
        ch->set_revent(m_arr_ev[i].events);
        vec_ch.push_back(ch);
    }
    
    return vec_ch;
}

bool Epoll::update_channel(Channel* ch)
{
    if (ch == nullptr)
        return false;

    struct epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->event();

    if (ch->inepoll())
    {
        if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, ch->fd(), &ev) == -1)
        {
            fprintf(stderr, "%s %d : %s \n", __FILE__, __LINE__, strerror(errno));
            return false;
        }
        return true;
    }
    else
    {
        if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, ch->fd(), &ev) == -1)
        {
            fprintf(stderr, "%s %d : %s \n", __FILE__, __LINE__, strerror(errno));
            return false;
        }
        ch->set_inepoll(true);
        return true;
    }    
}

bool Epoll::remove_channel(Channel* ch)
{
    if (ch == nullptr)
        return false;

    if (!ch->inepoll())
        return true;

    struct epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->event();

    if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, ch->fd(), &ev) == -1)
    {
        fprintf(stderr, "%s %d : %s \n", __FILE__, __LINE__, strerror(errno));
        return false;
    }
    ch->set_inepoll(false);
    return true;
}
