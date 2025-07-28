#pragma once

/*
    noncopyable ： 派生类是不能进行拷贝构造以及赋值但是可以进行构造以及析构
*/

class noncopyable{

    public:
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    protected:
        noncopyable() = default;
        ~noncopyable() = default;
};
