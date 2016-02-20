//
// Created by weitao on 16-2-10.
//

#ifndef BOOST_TEST1_EPOLL_COROUTINE_H
#define BOOST_TEST1_EPOLL_COROUTINE_H


#include <cstdint>
#include <memory>
#include <functional>
#include <map>
#include <set>
#include <vector>
#include <boost/noncopyable.hpp>
#include "event.hpp"
#include "coroutine.hpp"


struct epoll_event;

class CEpoll : public boost::noncopyable
{
public:

    CEpoll();

    ~CEpoll();

    static CEpoll& instance_();

    static coro_call_t& instance();

    int add_event(int fd, uint32_t mask, std::shared_ptr<coro_call_t> &coro, uint32_t time_out);

    int del_event(int fd, uint32_t mask);

    void poll(coro_yield_t &yield);

    void run();

    int64_t current_time() const;


private:
    using event_t     = CEvent<coro_call_t >;
    using event_map_t = std::map<int, std::shared_ptr<event_t>>;
    using event_set_t = std::set<std::shared_ptr<event_t>, std::less<std::shared_ptr<event_t>>>;
    int           m_epoll_fd;
    int           m_max_event;
    int           m_active_event_num;
    epoll_event * m_active_events;
    event_map_t   m_event_map;
    event_set_t   m_event_set;
};
/*
class CMainTask
{
public:

    static CMainTask& instance() {
        static CMainTask main_task;
        return main_task;
    }

    static coro_call_t &get_main_coro() {
        static coro_call_t the_main_coro(std::bind(&CMainTask::run, &instance(), std::placeholders::_1));
        return the_main_coro;
    }

    void run(coro_yield_t &yield) {
        while(true){
            auto new_event = yield.get();
            CEpoll::get_mutable_instance().add_event(new_event->fd, new_event);
            std::vector<std::shared_ptr<CEvent>> active_events;
            int active_event_num = CEpoll::get_mutable_instance().poll(&active_events, 1000);
            std::cout << "active num : " << active_event_num << std::endl;
            for(const auto &event : active_events) {
                CEpoll::get_mutable_instance().del_event(event->fd, event);
                yield(*(event->coro), event);
            }
        }
    }
};*/


#endif //BOOST_TEST1_EPOLL_COROUTINE_H
