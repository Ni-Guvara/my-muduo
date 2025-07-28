#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg)
    : baseLoop_(baseLoop)
    , name_(nameArg)
    , started_(false)
    , threadNum_(0)
    , next_(0) 
{

}

EventLoopThreadPool::~EventLoopThreadPool()
{
    
}

void EventLoopThreadPool::setThreadNum(int threadNum)
{
    threadNum_ = threadNum;
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    started_ = true;

    for(int i = 0; i < threadNum_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d",name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }

    if(threadNum_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

// 工作在多线程中， baseLoop_默认以轮询的方式将channel分配给subloop     
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseLoop_;

    if(!loops_.empty())
    {
        loop = loops_[next_];
        next_++;
        if(next_ >= loops_.size())
            next_ = 0;
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    EventLoop* loop = baseLoop_;

    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1, loop);
    }else
    {
        return loops_;
    }
}
