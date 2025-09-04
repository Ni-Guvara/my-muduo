#pragma once

#include "noncopyable.h"
#include "Thread.h"

#include <condition_variable>
#include <functional>
#include <string>
#include <mutex>


class EventLoop;

/**
 * 将EventLoop与Thread绑定在一起，One Thread One Loop
 * 
 */

class EventLoopThread : noncopyable
{
    public:
        using ThreadInitCallBack = std::function<void (EventLoop*)>;

        EventLoopThread(const ThreadInitCallBack& cb, const std::string& name = std::string());
        ~EventLoopThread();

        EventLoop* startLoop();
    private:

        void  ThreadFunc();

        EventLoop* loop_;
        bool exiting_;
        Thread thread_;
        std::mutex mutex_;
        std::condition_variable cond_;
        ThreadInitCallBack callback_;
};