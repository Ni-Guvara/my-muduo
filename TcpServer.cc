#include "Logger.h"
#include "TcpServer.h"
#include "TcpConnection.h"

#include <functional>
#include <string.h>

EventLoop* CheckNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is NULL!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const std::string& name, const InetAddress& listenAddr, Option option)
        : loop_(CheckNotNull(loop))
        , name_(name)
        , ipPort_(listenAddr.toIpPort())
        , acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)) 
        , threadPool_(new EventLoopThreadPool(loop_, name_))
        , connectionCallBack_()
        , messageCallBack_()
        , nextConnId_(1)
{
    //  当有新连接执行TcpServer::newConnection
    acceptor_->setNewConnectionCallBack(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for(auto &item :connections_)
    {
        TcpConnectionPtr conn(item.second);  // 这个局部的shared_ptr 出有括号可以直接释放new出来的TcpConnection的资源
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
    }
}

// 设置底层subloop个数
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

// 开启服务器监听
void TcpServer::start()
{
    if(started_++ == 0)  // 防止TcpServer被启动多次
    {
        threadPool_->start(threadInitCallBack_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}


// 跟据轮询算法选择一个subloop，唤醒subloop，把当前connfd封装的channel分发给subloop
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    // 轮询算法，选择一个subloop，来管理channel
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer:TcpConnection [%s] - new connection [%s] from %s \n",
        name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    // 通过sockfd获取绑定本机ip地址以及端口号
    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }

    InetAddress localAddr(local);

    // 根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop,
        connName,
        sockfd,
        localAddr,
        peerAddr
    ));

    connections_[connName] = conn;
    // 下面回调都是用户设置给TcpServer=》TcpConnection=》channel=》Poller注册Channel=》notify Channel回调
    conn->setConnectionCallBack(connectionCallBack_);
    conn->setMessageCallBack(messageCallBack_);
    conn->setWriteCompleteCallBack(writeCompleteCallBack_);


    conn->setCloseCallBack(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)
    );

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n", 
    name_.c_str(), conn->name().c_str());


    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();

    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn)); 
}
