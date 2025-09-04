#pragma once

#include "CurrentThread.h"
#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>


class Channel;
class Poller;

// 事件循环类，包含两个大模块 Channel Poller(epoll)

class EventLoop : noncopyable
{
    public:
        using Functor = std::function<void()>;
        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        void runInLoop(Functor cb);  // 当前线程中执行cb
        void queueInLoop(Functor cb); // 将cd放入队列，唤醒loop所在线程执行cb

        void wakeUp();             // 唤醒loop所在线程

        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

        bool isInThreadLoop(){ return threadId_ == CurrentThread::tid();}

        Timestamp pollReturnTime(){ return pollReturnTime_;};

    private:

        void handleRead();
        void doPendingFunctor();

        using ChannelList = std::vector<Channel*>;

        std::atomic_bool looping_;                // 标志loop循环通过CAS实现
        std::atomic_bool quit_;                   // 退出loop循环
        const pid_t threadId_;                    // 记录当前线程id

        Timestamp pollReturnTime_;                // poll返回channel的时间点
        std::unique_ptr<Poller>   poller_;
        
        int wakeUpFd_;                            // 主要作用，当MainLoop 获取一个新用户的Channel，通过轮询算法获取一个subloop，通过唤醒的subloop处理channel
        std::unique_ptr<Channel> wakeUpChannel_;  // 

        ChannelList activeChannel_;               // 当前活跃的Channel
        Channel* currentActiveChannel_;

        std::atomic_bool callingPendingFuncor_;    // 标志当前是否有需要执行的回调操作
        std::vector<Functor>  pendingFunctor_;    // 存储所有需要执行的回调操作
        std::mutex mutex_;                        // 使用锁来保证std::vector的线程安全
};