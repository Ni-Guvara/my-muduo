#pragma once

#include "noncopyable.h"
#include <functional>
#include <thread>
#include <memory>
#include <atomic>
#include <string>

class Thread
{
    public:
        using ThreadFunc = std::function<void()>;

        Thread(ThreadFunc func, const std::string name = std::string());
        ~Thread();

        void start();
        void join();

        bool started() const{ return started_;}
        bool joined() const {return joined_;}
        pid_t tid() const{ return tid_;}
        std::string& name(){ return name_; }
        int numCreated() { return numCreated_; } 

    private:
        
        void setDefaultName();
        
        std::shared_ptr<std::thread> thread_;
        pid_t  tid_;
        std::string name_;
        ThreadFunc  func_;     // 函数对象用来存储线程函数

        bool started_;
        bool joined_;

        static std::atomic_int32_t numCreated_;
};