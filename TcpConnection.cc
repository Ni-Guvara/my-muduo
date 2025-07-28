#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <errno.h>
#include <string>


static EventLoop* CheckNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnectionloop is NULL!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, 
                    const std::string& nameArg,
                    int sockfd,
                    const InetAddress& localAddr,
                    const InetAddress& peerAddr)
    : loop_(CheckNotNull(loop))
    , name_(nameArg)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64 * 1024 * 1024)
{

    // Channel 设置相应的回调函数，Poller通知Channel感兴趣的事件发生，Channel会调用相应的回调函数
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

void TcpConnection::send(const std::string& buf)
{
    if(state_ == kConnected)
    {
        if(loop_->isInThreadLoop())
        {
            sendInLoop(buf.c_str(), buf.size());
        }else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }    
    }
}


/**
 * 发送数据，应用写的快，内核发送数据慢，需要把待发送的数据写入缓冲区， 并且设置水位回调
 */
void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faulterror = false;


    // 之前调用过该Connection的shutdown， 不能再进行发送了
    if(state_ == KDisConnected)
    {
        LOG_ERROR("disconnected, give up writing!");
        return;
    }

    // channel第一次开始写数据，而且缓冲区没有发送数据
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if(nwrote > 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallBack_)
            {
                // 既然这里的数据全部发送完成，那么久不用再给channel设置epollout事件了
                loop_->queueInLoop(std::bind(writeCompleteCallBack_, shared_from_this()));
            }
        }else // nwrote < 0
        {
            nwrote = 0;
            if(errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop!");
                if(errno = EPIPE || errno == ECONNRESET)
                {
                    faulterror = true;
                }
            }
        }
    }

    /**
     * 说明当前这次write并没有将全部数据发送出去，剩余的数据需要保存到缓冲区之中，之后为channel是注册epollout事件
     * poller发现tcp发送缓冲区有空间，会通知相应的sock-channel ，调用writeCallBack_的回调方法，最终也就是调用
     * TcpConnection：：handlewrite方法， 把缓冲区的剩余数据全部发送完成
     */

    if(!faulterror && remaining > 0) 
    {
        size_t oldLen = outputBuffer_.readableBytes();

        if(oldLen + remaining >= highWaterMark_ 
            && oldLen < highWaterMark_
            && highWaterMarkCallBack_)
            {
                loop_->queueInLoop(std::bind(highWaterMarkCallBack_, shared_from_this(), oldLen + remaining));
            }
        outputBuffer_.append((char*)data + nwrote, remaining);

        if(!channel_->isWriting())
        {
            channel_->enableWriting(); // 注册channel的写事件，否则Poller不会给Chnnel 通知EPOLLOUT
        }
    }
}


// 关闭连接
void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisConnecting);
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this)
        );
    }
}


void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting())  // 说明当前outputbuffer数据全部发送完成
    {
        socket_->shutdownWrite(); // 关闭写端，触发EPOLLHUP 调用closecallback
    }
}


void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    // 新连接建立 执行回调
    connectionCallBack_(shared_from_this());
}

void TcpConnection::connectDestoryed()
{
    if(state_ == kConnected)
    {
        setState(KDisConnected);
        channel_->disableAll();  // 将channel感兴趣的事件，从poller中删除掉
        connectionCallBack_(shared_from_this());
    }
    channel_->remove();  // 将channel从poller中删除掉
}

void TcpConnection::handleRead(Timestamp receTimeStamp)
{
    int savedError = 0;
    size_t n = inputBuffer_.readFd(channel_->fd(), &savedError);
    if(n > 0)
    {
        // 已建立连接的用户，有可读事件发生了，调用用户传入的回调操作 onMessage
        messageCallBack_(shared_from_this(), &inputBuffer_, receTimeStamp);    

    }else if( 0 == n)
    {
        handleClose();
    }else
    {
        errno = savedError;
    }
}

void TcpConnection::handleWrite()
{
    if(channel_->isWriting())
    {
        int savedError = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedError);
        if(n > 0)
        {
            outputBuffer_.retrieve(n);

            if(outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                loop_->queueInLoop(std::bind(writeCompleteCallBack_, shared_from_this()));
            }

            if(state_ = kDisConnecting)
            {
                shutdownInLoop();
            }
        }else
        {
            LOG_ERROR("Error handleWrite\n");
        }
    }else
    {
        LOG_ERROR("Connection fd = %d is down, np more writing\n", channel_->fd());
    }
}
        
void TcpConnection::handleClose()
{
    LOG_INFO("fd= %d state= %d \n", channel_->fd(), (int)state_);
    setState(KDisConnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallBack_(connPtr);
    closeCallBack_(connPtr);
}
        
void TcpConnection::handleError()
{
    int optVal, err;
    socklen_t optlen = sizeof optVal;
    ssize_t n = ::setsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optVal, optlen);
    if(n < 0 )
    {   
        err = errno;
    }else
    {
        err = optVal;
    } 
    LOG_ERROR("TcpConnection::handleError: name:%s SO_ERROR %d\n", name_.c_str(), err);
}
        
TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd = %d state = %d\n", name_.c_str(), channel_->fd(), (int)state_);
}