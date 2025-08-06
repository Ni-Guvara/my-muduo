#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

static int createNonBlocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
} 

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr, bool reusePort)
    : loop_(loop)
    , acceptSocket_(createNonBlocking())
    , acceptChannel_(loop_, acceptSocket_.fd())
    , listening_(false) 
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(addr);  // 绑定地址

    // TcoServer::start() Acceptor.listen 有新用户连接，需要执行一个回调，将客户端连接的fd打包成Channel唤醒subLoop
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));  // 注册ReadCallBack
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}
       

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();  // listen
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);

    if(connfd > 0)
    {
        if(newConnectionCallBack_)
        {
            newConnectionCallBack_(connfd, peerAddr);  // 轮询找到唤醒subloop，分发当前的新客户端的Channel
        }else
        {
            ::close(connfd);
        }
    }else
    {
        LOG_ERROR("%s:%s:%d accept error %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if(errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reach limit!\n", __FILE__, __FUNCTION__, __LINE__);    
        }
    }
}