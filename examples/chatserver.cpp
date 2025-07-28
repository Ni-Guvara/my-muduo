#include <muduo/base/Logging.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

/* 使用muduo网络库创建服务器
    1、服务器使用TcpServer和事件循环EventLoop构建
    2、ChatServer的构造函数设计
    3、连接事件的回调函数设置onConnect和读写事件的回调事件onMessage
    4、设置合适的线程数量

*/
class ChatServer
{

public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg) : _server(loop, listenAddr, nameArg)
    {
        _loop = loop;

        // 设置连接的回调事件
        _server.setConnectionCallback(std::bind(&ChatServer::onConnect, this, std::placeholders::_1));

        // 设置读写的回调事件
        _server.setMessageCallback(std::bind(&ChatServer::onMeaasage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _server.setThreadNum(4);
    }

    void start()
    {
        _server.start();
    }

private:
    void onConnect(const TcpConnectionPtr &conn)
    {
        LOG_INFO << "Server -" << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort()
                 << " is " << (conn->connected() ? "UP" : "DOWN");
    }
    void onMeaasage(const TcpConnectionPtr &conn,
                    Buffer *buf,
                    Timestamp time)
    {
        muduo::string msg(buf->retrieveAllAsString());
        LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
                 << "data arrvied at " << time.toFormattedString();
        conn->send(msg);
    }

    TcpServer _server;
    EventLoop *_loop;
};

int main(int argc , char ** argv)
{
    muduo::net::EventLoop loop;
    muduo::net::InetAddress  listenAddress(8888);
    ChatServer server(&loop, listenAddress, "chatserver");
    server.start();
    loop.loop();
}