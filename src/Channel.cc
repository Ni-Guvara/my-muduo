#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel(EventLoop *loop, int fd) 
:   loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false)
{
    
}

Channel::~Channel(){ 
}

// Channel的tie方法什么时候调用过
void Channel::tie(const std::shared_ptr<void>& obj){
    tie_ = obj;   // 强弱智能指针的应用
    tied_ = true;
}

// 当改变Channel所在fd表示的events事件后，update 负责在epoller里面改变相应的事件epoll_ctl
// EventLoop -> ChannelList、Poller

void  Channel::update(){  

    // 通过Channel所属的EventLoop调用Poller相关方法， 注册的fd的events事件
    // add code...
    loop_->updateChannel(this);
}

// 在Channel所属的EventLoop中将当前的Channel删除掉
void Channel::remove(){
    // add code ...
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime){

    if(tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();  // 弱智能指针提升为强智能指针
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }else
    {
        handleEventWithGuard(receiveTime); 
    }
}


// 根据poller通用的发生的具体事件由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp  receiveTime){

    LOG_INFO("channel Events revents:%d", revents_);
    
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(closeCallBack_)
        {
            closeCallBack_();
        }
    }

    if((revents_ & EPOLLERR))
    {
        if(errorCallBack_)
        {
            errorCallBack_();
        }
    }

    if((revents_ & (EPOLLIN | EPOLLPRI)))
    {
        if(readCallBack_)
        {
            readCallBack_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT)
    {
        if(writeCallBack_)
        {
            writeCallBack_();
        }
    }
}
