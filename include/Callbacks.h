#pragma once

#include "Timestamp.h"

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;


using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallBack = std::function<void (const TcpConnectionPtr&)>;
using CloseCallBack = std::function<void (const TcpConnectionPtr&)>;
using WriteCompleteCallBack = std::function<void (const TcpConnectionPtr&)>;

using MessageCallBack = std::function<void (const TcpConnectionPtr&, Buffer*, Timestamp)>;
using HighWaterMarkCallBack = std::function<void(const TcpConnectionPtr&, size_t)>;
