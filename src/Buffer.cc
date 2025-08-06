#include "Buffer.h"

#include <unistd.h>
#include <sys/socket.h>

#include <sys/uio.h>

/**
 *  从fd上读取数据 Poller工作在LT模式
 *  Buffer缓冲区是有大小的！但是从fd上读取数据的时候食不知TCP数据大小
 * 
 */
size_t Buffer::readFd(int fd, int* saveErrno)
{
    char extraBuf[65536] = {0}; // 栈上内存 64K, 函数执行完直接释放

    struct iovec vec[2];

    size_t writeable = writeableBytes();

    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writeable;

    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    const int iovcnt = (writeable < sizeof extraBuf) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, iovcnt);

    if( n < 0 )
    {
        *saveErrno = errno;
    }else if( n <= writeable)
    {
        writeIndex_ += n;
    }else
    {
        writeIndex_ = buffer_.size();
        append(extraBuf, n - writeable);
    }
    return n;
}

size_t Buffer::writeFd(int fd, int* saveError)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if( n <= 0)
    {
        *saveError = errno;
    }
    return n;
}