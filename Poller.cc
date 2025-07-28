#include "Poller.h"


Poller::Poller(EventLoop *loop) : ownerLoop_(loop){}

Poller::~Poller() = default;


bool Poller::hasChannel(Channel* channel) const{
    auto it = channels_.find(channel->fd());
    
    if(it != channels_.end())
    {
        if(it->second == channel)
            return true;
    }

    return false;
}