#ifndef __timer_hh__
#define __timer_hh__

#include "object.h"

namespace kkit
{

// class Poller;

class timer : public object, public std::enable_shared_from_this<timer>
{    
public:
    typedef std::function<void (std::shared_ptr<timer> ptr)> cb; 
protected:
    timer();
    ~timer();
    friend class poller;
public:
    virtual int start() override;
    virtual int stop() override;
protected:
    virtual void callback(int ev) override;
public:
    void set(std::weak_ptr<poller> p) { _poller = p;}
    void set(timer::cb cb) {_cb = cb; }
    void set(long long first, long long interval); //单位是毫秒 millisecond
public:
    int fd(){return _fd;}
    std::weak_ptr<Poller> poller(){return _poller;}
private:
    timer::cb _cb = nullptr;
    std::weak_ptr<poller> _poller;;
    struct itimerspec _iti;
};
}