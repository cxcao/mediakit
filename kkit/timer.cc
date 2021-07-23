#include "timer.h"
#include "poller.h"

namespace kkit
{


timer::timer() : object(object::type::timer)
{
    _fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
}
timer::~timer()
{
    if(_fd > 0) close(_fd);
}

int timer::stop()
{
    auto ptr = _poller.lock();
    if(ptr) ptr->stop(std::dynamic_pointer_cast<object>(shared_from_this()));
    return 0;
}
int timer::start()
{
    auto ptr = _poller.lock();
    if(!ptr) return -1;

    timerfd_settime(_fd, 0, &_iti, NULL);
    ptr->set(std::dynamic_pointer_cast<object>(shared_from_this()));
    return 0;
}


void timer::set(long long first, long long interval)
{
    _iti.it_value.tv_sec = first / 1000;
    _iti.it_value.tv_nsec = (first % 1000) * 1000000; 

    _iti.it_interval.tv_sec = interval / 1000;
    _iti.it_interval.tv_nsec = (interval % 1000) * 1000000;
}

void timer::callback(int ev)
{   
    uint64_t exp;
    ssize_t n = read(_fd, &exp, sizeof(uint64_t));

    if(_cb) _cb(shared_from_this());
}
}