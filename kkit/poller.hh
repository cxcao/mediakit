#ifndef __poller__hh__
#define __poller__hh__

namespace kkit {


class poller
{
    typedef std::function<void (int ev)> cb;
    typedef std::shared_ptr<poller> ptr;
public:
    static poller::ptr get_ptr();
protected:
    ~poller();
    poller();

public:
    
    int set(std::shared_ptr< class object > ptr);
    int stop(std::shared_ptr< class object > ptr);

    int run();
    int stop();
    
private:
    int ctl(std::shared_ptr<void> ptr, poller::cb cb);
private:
    int _eventfd = 0, _fd = 0;
    pthread_t _id;
    std::unordered_map< int,  cb > _ev_map;
    std::future<void> _future;
    std::atomic<bool> _running(false);
    std::shared_mutex _mutex;
};

};

#endif