## 剖析muduo库的核心源码
### EventLoop
    1、将Channel送到Epoller中，通过Epoller修改epoll感兴趣的事件(update),从Epoller中获取活跃的Channel(poll)
    2、在loop中运行活跃Channel中的回调函数
### EventLoopThread
    1、threadHandler中运行EventLoop的loop
### EventLoopThreadPool
    1、根据threadNum 来运行threadsNum个线程，线程中运行EventLoop的loop(One Loop Per Thread)
    2、通过getNextLoop轮询获取sub EventLoop
### Epoller
    1、将新的Channel(fd)注册到epoll，也可删除、修改
    1、将Channel感兴趣的事件在epoll中修改
### Channel
    1、封装了文件描述符、感兴趣的事件（读、写）、返回的事件
    2、执行回调
### Acceptor
    1、在Main EventLoop中将监听新来的连接，并将事件封入Channel
    2、将新连接轮询分发给Sub EventLoop
### TcpConnection
### TcpServer
    1、Acceptor、threadPool_