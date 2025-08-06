#include "InetAddress.h"

#include <strings.h>
#include <string.h>
#include <iostream>

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof addr_);

    addr_.sin_family = AF_INET;       // 协议簇
    addr_.sin_port = htons(port);     // 端口
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); // ip地址
}

std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    int end = strlen(buf);
    sprintf(buf + end, ":%u", ntohs(addr_.sin_port));
    return buf;
}

/*
* 测试
*/
// int main()
// {
//     uint16_t port = 1234;
//     InetAddress addr(1234);
//     std::cout << "Ip : " << addr.toIp() << std::endl;
//     std::cout << "Port : " << addr.toPort() << std::endl;
//     std::cout << "IpPort : " << addr.toIpPort() << std::endl;

//     return 0;

// }