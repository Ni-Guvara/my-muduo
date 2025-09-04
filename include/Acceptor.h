#pragma once

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h" 

#include <functional>

class EventLoop;

class Acceptor: noncopyable{

    public:
       using NewConnectionCallBack = std::function<void(int sockfd, InetAddress& addr)> ;
       Acceptor(EventLoop* loop, const InetAddress& addr, bool reusePort);
       ~Acceptor();
        
       void setNewConnectionCallBack(const NewConnectionCallBack& cb)
       {
            newConnectionCallBack_ = cb;
       }

       bool listening() const { return listening_;}
       void listen(); 

    private:
        void handleRead();

        EventLoop* loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallBack newConnectionCallBack_;
        bool listening_;
};