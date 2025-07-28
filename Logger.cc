#include "Logger.h"


Logger &Logger::instance()
{
    static Logger logger;    // 线程安全
    return logger;
}

// 设置日志级别
void Logger::setLogLevel(int level)
{
    logLevel_ = level;
}

// 写日志, 先打印级别信息，打印日志时间信息，再打印具体内容
void Logger::log(std::string msg)
{
    // 打印级别信息
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }

    // 打印时间
    std::cout << "print time : "  << msg << std::endl;

}