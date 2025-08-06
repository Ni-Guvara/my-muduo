#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>

#include <netinet/tcp.h>




Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localAddr)
{
    if(::bind(sockfd_, (sockaddr*)localAddr.getSockAddr(), sizeof(sockaddr_in)) != 0)
    {
        LOG_FATAL("bind sockfd: %d fail\n", sockfd_);
    }
}

void Socket::listen()
{
    if(0 != ::listen(sockfd_, 1024))
    {
        LOG_FATAL("listen fd:%d fail\n", sockfd_);
    }
}

int Socket::accept(InetAddress* peeraddr)
{
    socklen_t len;
    sockaddr_in addr;
    bzero(&addr, sizeof addr);

    int connfd = ::accept(sockfd_, (sockaddr*)&addr, &len);

    if(connfd > 0)
    {
        peeraddr->setSockAddr(addr);
    }

    return connfd;
}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR("shutdown write fail\n");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int valopt = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY , &valopt, sizeof valopt);  // 协议级别
}

void Socket::setReuseAddr(bool on)
{
    int valopt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &valopt, sizeof valopt);  // socket级别
}

void Socket::setReusePort(bool on)
{
    int valopt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &valopt, sizeof valopt);
}

void Socket::setKeepAlive(bool on)
{
    int valopt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &valopt, sizeof valopt);
}