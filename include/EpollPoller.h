#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <sys/epoll.h>

class Channel;

/**
 * epoll的使用
 * epoll_create
 * epoll_ctl       add/mod/del
 * epoll_wait
 *  
 */
class EpollPoller : public Poller{
    public:
        EpollPoller(EventLoop *loop);
        ~EpollPoller();

        Timestamp poll(int timeouts, ChannelList *activateChannels)override;

        void updateChannel(Channel* channel)override;

        void removeChannel(Channel* channel)override;

    private:

        using EventList = std::vector<struct epoll_event>;
        
        static const int kInitEventListSize = 16;

        void fillActiveChannels(int numEvents, ChannelList* activeChannels)const;  // 填写活跃的Channel

        void update(int operation, Channel* channel);

        int epollfd_;
        EventList events_;
};