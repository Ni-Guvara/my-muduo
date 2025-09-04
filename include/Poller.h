#pragma

#include "noncopyable.h"
#include "Timestamp.h"
#include "Channel.h"

#include <vector>
#include <unordered_map>


class EventLoop;

/**
 *  muduo库中多路事件分发器的核心
 *
 */

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;
    Poller(EventLoop *loop);
    virtual ~Poller();

    // 给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeouts, ChannelList *activateChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    // 判断参数Channel是否在当前Poller中
    virtual bool hasChannel(Channel *channel)const; 

    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    // map的int即为sockfd value即为Channel类型 
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; // 定义poller所属的事件循环发生器
};
