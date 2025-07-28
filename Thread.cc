#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic_int32_t Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string name)
        :tid_(0)
        ,func_(std::move(func))
        ,name_(name)
        ,started_(false)
        ,joined_(false)
        
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();  // thread提供设置分离线程的方法，设置成分离线程
    }
}

void Thread::start()   //  一个thread对象记录的就是一个线程的详细信息
{
    started_ = true;

    sem_t sem;  // 使用信号量保证tid的安全赋值
    sem_init(&sem, false, 0);    
    
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){

        // 获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        // 开启一个新线程，执行线程函数
        func_();
    }));

    // 这里必须等待上面的线程得到tid值
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = numCreated_++;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}