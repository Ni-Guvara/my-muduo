#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop; // 前置声明暴露的头文件信息少

// 理清EventLoop、Poller、Channel之间的关系 <= Reactor模型与Demultiplex
// Channel 理解为通道封装了sockfd以及其感兴趣的事件，例如EPOLLIN、EPOLLOUT事件。还绑定了poller返回的具体事件
class Channel : noncopyable
{

public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd 得到poller通知后，执行的
    void handleEvent(Timestamp receiveTime);

    void handleEventWithGuard(Timestamp revieveTime);

    // 设置回调函数的对象
    void setReadCallback(ReadEventCallBack cb) { readCallBack_ = std::move(cb); }

    // 设置写回调函数对象
    void setWriteCallBack(EventCallBack cb) { writeCallBack_ = std::move(cb); }

    // 设置关闭回调函数对象
    void setCloseCallBack(EventCallBack cb) { closeCallBack_ = std::move(cb); }

    // 设置出错回调函数对象
    void setErrorCallBack(EventCallBack cb) { errorCallBack_ = std::move(cb); }

    // 防止channel被remove掉，channel还执行回调函数
    void tie(const std::shared_ptr<void>&);

    int fd()const{return fd_;}

    int events()const{return events_;}

    void set_revents(int revt){ revents_ = revt;}

    bool isNonEvent() const {return events_ == kNoneEvent;}

    // 设置fd相应的事件
    void enableReading() {events_ |= kReadEvent;update();}

    void disableReading() {events_ &= ~kReadEvent;update();}

    void enableWriting() {events_ |= kWriteEvent;update();}

    void disableWriting() {events_ &= ~kWriteEvent;update();}

    void disableAll() {events_ = kNoneEvent ;update();}

    bool isNoneEvent() const {return events_ == kNoneEvent;}

    bool isReading() const { return events_ & kReadEvent;}

    bool isWriting() const { return events_ & kWriteEvent;}

    int index() const{return index_;}
    
    void set_index(int idx) { index_ = idx;}

    // one loop per thread
    EventLoop* ownerLoop(){ return loop_;}

    void remove();

private:
    void update(); // 当改变Channel所在fd表示的events事件后，update 负责在epoller里面改变相应的事件epoll_ctl;

    static const int kNoneEvent;
    static const int kWriteEvent;
    static const int kReadEvent;

    EventLoop *loop_; // 事件循环
    const int fd_;    // Poller监听的对象
    int events_;      // 注册fd感兴趣的事件
    int revents_;     // epoller返回的具体事件
    int index_;       // kNew、kAdded、kDeleted

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;
};
