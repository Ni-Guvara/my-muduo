#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;


/**
 * TcpServer => Acceptor => 有一个新用户的链接，通过accept函数拿到connfd
 * ==》TcpConnection设置回调 =》 Channel => Poller => Channel的回调操作
 */
class TcpConnection : noncopyable,  public std::enable_shared_from_this<TcpConnection>
{
    public:
        TcpConnection(EventLoop* loop, 
                    const std::string& name,
                    int sockfd,
                    const InetAddress& lcoalAddr,
                    const InetAddress& peerAddr);
        
        ~TcpConnection();

        EventLoop* getLoop() const{ return loop_;}
        const std::string& name() const{ return name_;}
        const InetAddress& localAddress() const { return localAddr_;}
        const InetAddress& peerAddress() const{return peerAddr_;}
        bool connected() { return state_ == kConnected;}

        void send(const std::string& buf);

        void shutdown();  // 关闭连接
        void shutdownInLoop();

        void setConnectionCallBack(const ConnectionCallBack& cb)
        {
            connectionCallBack_ = cb;
        }

        void setMessageCallBack(const MessageCallBack& cb)
        {
            messageCallBack_ = cb;
        }

        void setWriteCompleteCallBack(const WriteCompleteCallBack& cb)
        {
            writeCompleteCallBack_ = cb;
        }

        void setHighWaterMarkCallBack(const HighWaterMarkCallBack& cb, size_t highWaterMark)
        {
            highWaterMarkCallBack_ = cb;
            highWaterMark_ = highWaterMark;
        }

        void setCloseCallBack(const CloseCallBack& cb)
        {
            closeCallBack_ = cb; 
        }

        void connectEstablished();

        void connectDestoryed();


    private:

        enum StateE{KDisConnected, kConnecting, kConnected, kDisConnecting};
        void handleRead(Timestamp timestamp);
        void handleWrite();
        void handleClose();
        void handleError();

        void sendInLoop(const void* data, size_t len);
        

        void setState(StateE s){ state_ = s;}

        EventLoop* loop_;  // TcpConnection都是在subloop里面管理的
        const std::string name_;
        bool reading_;
        std::atomic_int state_;

        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;

        const InetAddress localAddr_;
        const InetAddress peerAddr_;

        ConnectionCallBack connectionCallBack_;            // 新连接时的回调
        MessageCallBack  messageCallBack_;                 // 有读写消息时的回调
        WriteCompleteCallBack writeCompleteCallBack_;      // 消息发送完成以后的回调
        HighWaterMarkCallBack highWaterMarkCallBack_;
        CloseCallBack   closeCallBack_;
        size_t highWaterMark_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;
};