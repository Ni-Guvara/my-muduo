#include "EpollPoller.h"
#include "Logger.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>

const int kNew     =  -1;  // channel 未添加到Poller中
const int kAdded   =   1;  // Channel 已添加到Poller中
const int kDeleted =   2;  // Channel 从Poller中删除

EpollPoller::EpollPoller(EventLoop* loop) 
        : Poller(loop)
        , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
        , events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_create fatal:%d\n", errno);
    }
    
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}


Timestamp EpollPoller::poll(int timeouts, ChannelList *activateChannels){

    LOG_INFO("func=>%s, fd total size %ld",__FUNCTION__, events_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeouts);
    int savedError = errno;

    Timestamp now = Timestamp::now();

    if(numEvents > 0)
    {
        LOG_INFO("%d events happen\n", numEvents);
        fillActiveChannels(numEvents, activateChannels);

        // 如果发生事件大于数组大小则需要扩容
        if(numEvents == static_cast<int>(events_.size()))
        {
            events_.resize(numEvents * 2);
        }

    }else if(numEvents == 0)
    {
        LOG_INFO("no events happen\n");
    }else
    {
        if(savedError != EINTR)
        {
            errno = savedError;
            LOG_ERROR("EpollPoller::poll()");
        }
    }
    return now;
}

/**
 *                    EventLoop
 *      ChannelList                Poller
 *                                      ChannelMap<fd, Channel*>
 * 
 * 
 */

void EpollPoller::updateChannel(Channel* channel){

    const int index = channel->index();
    LOG_INFO("func = %s => fd = %02d , events = %02d , index = %02d\n",__FUNCTION__, channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted)
    {
        if(index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }else
        {
            // 已经被删除
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }else
    {
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel){

    int fd = channel->fd();
    channels_.erase(fd);

    int index = channel->index();    
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 填写活跃的Channel
void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels)const{

    for(int i = 0 ;i < numEvents; i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::update(int operation, Channel* channel){

    epoll_event event;
    int fd = channel->fd();
    bzero(&event, sizeof event);

    event.events = channel->events();
    event.data.fd = channel->fd();
    event.data.ptr = channel;

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll ctl del error : %d\n", errno);
        }else
        {
            LOG_FATAL("epoll ctl del fatal : %d\n", errno);
        }
    }
}