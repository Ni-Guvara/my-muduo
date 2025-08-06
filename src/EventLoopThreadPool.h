#pragma once

#include "noncopyable.h"

#include <functional>
#include <vector>
#include <string>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public noncopyable{
    
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;
        EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg = std::string());
        ~EventLoopThreadPool();

        void setThreadNum(int threadNum);
        void start(const ThreadInitCallback& cb = ThreadInitCallback());

        // 工作在多线程中， baseLoop_默认以轮询的方式将channel分配给subloop     
        EventLoop* getNextLoop();


        std::vector<EventLoop*> getAllLoops();

        bool started() const{ return started_;}
        const std::string& name() const{ return name_;}



    private:
        EventLoop* baseLoop_;
        std::string name_;
        bool started_;
        int threadNum_;
        int next_;
        
        std::vector<std::unique_ptr<EventLoopThread>> threads_;  
        std::vector<EventLoop*> loops_;
};