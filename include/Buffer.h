#pragma once

#include <vector>
#include <string>
#include <algorithm>


// 网络库底层的
class Buffer{
    public:
        static const size_t kCheapPrepend_ = 8;
        static const size_t kInitialSize_  = 1024;

        explicit Buffer(size_t initialSize = kInitialSize_)
            : buffer_(kCheapPrepend_ + initialSize)
            , readIndex_(kCheapPrepend_)
            , writeIndex_(kCheapPrepend_)
            {
                
            }

        size_t readableBytes() const{ return writeIndex_ - readIndex_;}

        size_t writeableBytes() const{ return buffer_.size() - writeIndex_;}

        size_t prependableBytes() const { return readIndex_;}

        // onMessage string <- Buffer
        void retrieve(size_t len)
        {
            if(len < readableBytes())
            {
                readIndex_ == len;  // 应用读取了可读缓冲区但没有读完
            }else
            {
                retrieveAll();
            }
        }

        void retrieveAll()
        {
            readIndex_ = writeIndex_ = kCheapPrepend_;
        }

        std::string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        std::string retrieveAsString(size_t len)
        {
            std::string result(peek(), len);
            retrieve(len);   // 上面缓冲区数据读出
            return result;  
        }

        // buffer_.size - writeerIndex_
        void ensureWriteableBytes(size_t len)
        {
            if(writeableBytes() < len)
            {
                makeSpace(len);
            }
        }

        void append(char* data, size_t len)
        {
            ensureWriteableBytes(len);
            std::copy(data, data + len , beginWrite()); // 拷贝完数据之后，移动写指针
            writeIndex_ += len;
        }

        // 从fd上读取数据
        size_t readFd(int fd, int* saveErrno);

        // 通过Fd发送数据
        size_t writeFd(int fd, int* saveErrno);

    private:

        char* begin()
        {
            // 调用迭代器 it.operator*()
            return &*buffer_.begin();
        }

        const char* begin() const
        {
            return &*buffer_.begin();
        }

        // 返回缓冲区中可读数据的起始地址
        const char* peek()const
        {
            return begin() + readIndex_;
        }

        void makeSpace(size_t len)
        {
            if( readIndex_ + writeableBytes() < kCheapPrepend_ + len)
            {
                buffer_.resize(writeIndex_ + len);
            }else
            {
                size_t readable = readableBytes();
                
                // 将数据前移
                std::copy(buffer_.begin() + readIndex_
                        , buffer_.begin() + writeIndex_
                        , buffer_.begin() + kCheapPrepend_);
                readIndex_ = kCheapPrepend_;
                writeIndex_ = readIndex_ + readable;

            }
        }
        
        char* beginWrite(){
            return begin() + writeIndex_;
        }
        const char* beginWrite() const
        {
            return begin() + writeIndex_;
        }
       


        std::vector<char> buffer_;
        size_t readIndex_;
        size_t writeIndex_;
};