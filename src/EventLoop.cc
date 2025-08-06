#include "EventLoop.h"
#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <memory>

__thread EventLoop* t_loopInThisThread = nullptr; // 防止一个线程中有多个EventLoop

const int kPollTimeMs = 10000; // 定义默认Poller IO复用超时时间

int creatEventFd()
{
    int ret =  ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(ret < 0)
    {
        LOG_FATAL("Failed in eventfd");
    }

    return ret;
}


EventLoop::EventLoop() 
              : looping_(false),
                quit_(false),
                callingPendingFuncor_(false),
                poller_(EpollPoller::newDefaultPoller(this)),
                threadId_(CurrentThread::tid()),
                wakeUpFd_(creatEventFd()),
                wakeUpChannel_(new Channel(this, wakeUpFd_)),
                currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in thread %d\n", this, threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in thread %d\n", t_loopInThisThread, threadId_);
    }else
    {
        t_loopInThisThread = this;
    }

    // 设置wakeupFd的事件类型以及事件发生的回调操作
    wakeUpChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeUpChannel_->enableReading();
}



void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping\n", this);

    while(!quit_)
    {
        activeChannel_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannel_);
        
        for(Channel* channel : activeChannel_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        // 事件循环需要处理的回调操作
        
        /**
         *   mainLoop accept fd =====> channel =====> subLoop 
         *   mainLoop 注册回调 subLoop 执行回调 
         */
        
        doPendingFunctor();
    }

    LOG_INFO("EventLoop %p stop looping. \n", this);
    looping_ = false;
}

// 1、自己线程调用quit 2、其他线程调用quit，当Loop在Poller_->poll 阻塞时，需要将该Loop进行唤醒，线程的循环才会退出
void EventLoop::quit()
{
    quit_ = true;

    if(!isInThreadLoop())
    {
        wakeUp();
    }

}

void EventLoop::runInLoop(Functor cb) // 当前loop线程中执行cb
{
    if(isInThreadLoop())  // 在当前线程中执行
    {
        cb();
    }else  // 在非当前loop线程中执行cb，需要唤醒loop所在线程，执行cb
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) // 将cd放入队列，唤醒loop所在线程执行cb
{
    { // 并发执行
        std::unique_lock<std::mutex> lock(mutex_); // 使用锁保证线程安全
        pendingFunctor_.emplace_back(cb);
    }

    // 唤醒执行上面执行对应loop的回调函数
    if(!isInThreadLoop() || callingPendingFuncor_)
    {
        wakeUp();
    }
}

void EventLoop::wakeUp()             // 唤醒loop所在线程
{
    uint64_t one = 1;
    int len = write(wakeUpFd_, &one, sizeof one);
    if(len != sizeof one)
    {
        LOG_ERROR("EventLoop::write() writes %d instead of 8 bytes\n", len);
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel){
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}


void EventLoop::handleRead()
{
    uint64_t one = 1;
    size_t n = read(wakeUpFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop handleRead() reads %lu instead of 8 bytes\n", n);
    }
}

void EventLoop::doPendingFunctor() // 执行回调
{
    std::vector<Functor> functors;
    callingPendingFuncor_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctor_);
    }

    for(Functor& functor : functors)
    {
        functor();
    }

    callingPendingFuncor_ = false;
}

EventLoop::~EventLoop()
{
    wakeUpChannel_->disableAll();
    wakeUpChannel_->remove();
    ::close(wakeUpFd_);
    t_loopInThisThread = nullptr;
}