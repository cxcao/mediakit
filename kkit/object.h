#pragma once

#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <chrono>
#include <thread>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h> 
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <memory>
#include <thread>
#include <functional>
#include <queue>
#include <stdint.h>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>
#include <future>
#include <sys/timerfd.h>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <vector>
#include <netdb.h>
#include <unordered_map>

namespace kkit
{

// enum EventType {
//     kRead = 1 << 0, //读事件
//     kWrite = 1 << 1, //写事件
//     kError = 1 << 2, //错误事件
//     kLT = 1 << 3,//水平触发
//     kUnknow = 0,
// };



class object
{
public:
    enum class type
    {
        timer,
        poller,
    };
public:
    object(enum type type) : _type(type) {}
    ~object(){}
    friend class poller;
public:
    virtual int start() = 0;
    virtual int stop() = 0;
    int fd() {return _fd;}
protected:
    virtual void callback(int ev) = 0;
    int _fd = 0;
public:
    enum type type() { return _type;}
private:
    enum type _type = type::timer;  
};

}
