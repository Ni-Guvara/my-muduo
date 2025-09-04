#pragma once

#include <string>
#include <iostream>

#include "noncopyable.h"

// INFO(%s, %d, arg1 , arg2)
#define LOG_INFO(logmsgFormat, ...)\
    do\
    {\
        Logger& logger = Logger::instance();\
        logger.setLogLevel(INFO);\
        char buf[1024];\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)

#define LOG_ERROR(logmsgFormat, ...)\
    do\
    {\
        Logger& logger = Logger::instance();\
        logger.setLogLevel(ERROR);\
        char buf[1024];\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)

#define LOG_FATAL(logmsgFormat, ...)\
    do\
    {\
        Logger& logger = Logger::instance();\
        logger.setLogLevel(FATAL);\
        char buf[1024];\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
        exit(-1);\
    }while(0)


#ifndef MDEBUG
#define LOG_DEBUG(logmsgFormat, ...)
#else
#define LOG_DEBUG(logmsgFormat, ...)\
    do\
    {\
        Logger& logger = Logger::instance();\
        logger.setLogLevel(DEBUG);\
        char buf[1024];\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)
#endif



// 定义日志级别 INFO   ERROR   FATAL   DEBUG
enum logLevel{
    INFO,
    ERROR,
    FATAL,
    DEBUG
};


// 定义日志类, 采用单例模式
class Logger : noncopyable{
    
    public:
        // 获取日志的单例
        static Logger& instance();
        // 设置日志级别
        void setLogLevel(int level);
        // 写日志
        void log(std::string msg);

    private:
        int logLevel_;       
        Logger() = default;
};