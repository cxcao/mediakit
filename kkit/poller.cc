#include "poller.h"
#include "timer.h"
// #include "io.h"

namespace ev
{

// #define toEpoll(event)    (((event) & kRead) ? EPOLLIN : 0) \
//                             | (((event) & kWrite) ? EPOLLOUT : 0) \
//                             | (((event) & kError) ? (EPOLLHUP | EpollerR) : 0) \
//                             | (((event) & kLT) ?  0 : EPOLLET)
// #define toLoop(epoll_event) (((epoll_event) & EPOLLIN) ? (int)kRead : 0) \
//                             | (((epoll_event) & EPOLLOUT) ? (int)kWrite : 0) \
//                             | (((epoll_event) & EPOLLHUP) ? (int)kError : 0) \
//                             | (((epoll_event) & EpollerR) ? (int)kError : 0)



std::shared_ptr<poller> poller::createShared(long unsigned evnum)
{
    class evEnableSharea : public poller
    {
    public:
        evEnableSharea(long unsigned evnum) : poller(evnum) {};
        ~evEnableSharea(){};
    };
    return std::make_shared<evEnableSharea>(evnum);
}

poller::poller(long unsigned int evnum) : _evnum(evnum)
{
    _fd = epoll_create(1024);
    _eventfd = ::eventfd(0,EFD_NONBLOCK | EFD_SEMAPHORE | EFD_CLOEXEC);
    _id = pthread_self();
}
poller::~poller()
{

}

int poller::run()
{
    for(int i = 0; i < 2; i++)
    {
        struct epoll_event ev = {0};
        ev.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
        ev.data.fd = _eventfd[i];
        epoll_ctl(_fd, EPOLL_CTL_ADD, _eventfd[i], &ev);
    }
   
   _running = true;
    _future = std::async(std::launch::async, [this]() {
        _id = pthread_self();
        while(this->_running)
        {
            struct epoll_event events[1024];
            int ret = epoll_wait(this->_fd, events, 1024, -1);
            for (int i = 0; i < ret; ++i) {
                if(events[i].data.fd == _eventfd[0])
                {
                    uint64_t count = 1;
                    std::lock_guard<std::mutex> lock(_mutex);
                    eventfd_read(events[i].data.fd, &count);
                    std::shared_ptr< class newsData > ptr = this->_ev_queue[0].front();
                    this->_ev_queue[0].pop();
                    ptr->cb(ptr->evptr);
                    continue;
                }

                auto it = this->_ev_map.find(events[i].data.fd);
                if (it == this->_ev_map.end()) {
                    epoll_ctl(this->_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL); continue;
                }
                auto cb = it->second;
                cb(events[i].events);
            }
        }
    });
    return 0;
}

int poller::stop()
{
    _running = false;

    std::future_status status;
    do {
        status = _future.wait_for(std::chrono::seconds(1));
        if (status == std::future_status::deferred) {
            printf("poller::stop: deferred\n");
        } else if (status == std::future_status::timeout) {
            printf("poller::stop: timeout\n");
        } else if (status == std::future_status::ready) {
            printf("poller::stop: ready!\n");
        }
    } while (status != std::future_status::ready); 

    return 0;
}
int poller::set(std::shared_ptr<void> ptr, newsCallBack cb)
{
    std::shared_ptr< class newsData > nptr = std::make_shared<class newsData>();
    nptr->evptr = ptr; nptr->cb = cb;

    uint64_t count = 1;
	std::lock_guard<std::mutex> lock(_mutex);
	if (_ev_queue[0].size() >= _evnum) return -1;
	
	eventfd_write(_eventfd[0], count);
    _ev_queue[0].push(nptr); 
    return 0;
}

int poller::ctl(std::shared_ptr<void> ptr, newsCallBack cb)
{
    std::shared_ptr< class newsData > nptr = std::make_shared<class newsData>();
    nptr->evptr = ptr; nptr->cb = cb;

    uint64_t count = 1;
	std::lock_guard<std::mutex> lock(_mutex);
	eventfd_write(_eventfd[1], count);
    _ev_queue[1].push(nptr);
    return 0;
}


int poller::set(std::shared_ptr< class object > ptr)
{
    if(ptr == nullptr) return -1;
    if(_id == pthread_self())
    { 
        struct epoll_event ev = {0};
        if(ptr->type() == object::type::timer) ev.events = EPOLLIN | EpollerR |  EPOLLEXCLUSIVE;
        if(ptr->type() == object::type::io) ev.events = EPOLLIN | EPOLLHUP | EpollerR | EPOLLET | EPOLLOUT | EPOLLEXCLUSIVE;
        
        ev.data.fd = ptr->fd();
        int ret = epoll_ctl(_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
        if(ret != 0) return ret;

        _ev_map.emplace((int)ev.data.fd,[ptr](int ev){
            ptr->callback(ev);
        });
        return 0;
    }
    
    poller::ctl(ptr, [this, ptr](std::weak_ptr<void> evptr){
    this->set(ptr);
    }
    );

    return 0;
}
int poller::stop(std::shared_ptr< class Object > ptr)
{
    if(_id == pthread_self())
    {  
        auto it = this->_ev_map.find(ptr->fd());
        if (it == this->_ev_map.end()) {
            std::cout << "Object: is not start" << std::endl; return 0;
        }
        epoll_ctl(_fd, EPOLL_CTL_DEL, ptr->fd(), NULL);
        this->_ev_map.erase(it);
        return 0;
    }

    poller::ctl(ptr, [](std::shared_ptr<void> evptr){
        ((Object *)evptr.get())->stop();
    });
    return 0;
}

}