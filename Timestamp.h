#pragma once
#include <iostream>
#include <string>

/*
*  时间类
*/
class Timestamp
{
public:
    Timestamp() = default;
    explicit Timestamp(int64_t secondsSinceEpochArg);   // 使用explicit明确表示不使用隐式转换

    static Timestamp now();
    std::string toString() const;

private:
    int64_t secondsSinceEpoch_;
};