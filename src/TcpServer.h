#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

/**
 * 用户使用muduo库编写服务器程序
 */

// TCP服务器类
class TcpServer : noncopyable
{
    public:
        using ThreadInitCallBack = std::function<void(EventLoop*)>;
        enum Option{
            kNoReusePort,
            kReusePort
        };

        TcpServer(EventLoop* loop, const std::string& name, const InetAddress& listenAddr, Option option = kNoReusePort);
        ~TcpServer();

        void setThreadInitCallBack(const ThreadInitCallBack& cb) { threadInitCallBack_ = cb;}
        void setConnectionCallBack(const ConnectionCallBack& cb) { connectionCallBack_ = cb;}
        void setMessageCakkBack(const MessageCallBack& cb){ messageCallBack_ = cb;}
        void setWriteCompleteCallBack(const WriteCompleteCallBack& cb){ writeCompleteCallBack_ = cb;}


        // 设置底层subloop个数
        void setThreadNum(int numThreads);

        // 开启服务器监听
        void start();

    private:
        void newConnection(int sockfd, const InetAddress &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

        using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
        
        EventLoop* loop_;
        const std::string ipPort_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;               // 运行在mainloop，主要用来监听新连接
        std::unique_ptr<EventLoopThreadPool> threadPool_;  // one loop one thread

        ConnectionCallBack connectionCallBack_;            // 新连接时的回调
        MessageCallBack  messageCallBack_;                 // 有读写消息时的回调
        WriteCompleteCallBack writeCompleteCallBack_;      // 消息发送完成以后的回调
       
        ThreadInitCallBack threadInitCallBack_;            // 线程初始化的回调
        
        std::atomic_int started_;
        int nextConnId_;
        ConnectionMap connections_;
};
