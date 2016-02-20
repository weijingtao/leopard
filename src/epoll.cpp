//
// Created by weitao on 16-2-10.
//

//
// Created by weitao on 8/25/15.
//
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <cstring>
#include <cassert>
#include <iostream>
#include <functional>
#include <chrono>
#include "epoll.h"

CEpoll& CEpoll::instance_() {
    static CEpoll the_epoll;
    return the_epoll;
}

coro_call_t& CEpoll::instance() {
    static coro_call_t the_epoll_coro(std::bind(&CEpoll::poll, &instance_(), std::placeholders::_1));
    return the_epoll_coro;
}

CEpoll::CEpoll()
        : m_epoll_fd(0),
          m_max_event(32768),
          m_active_event_num(5),
          m_active_events(nullptr)
{
    m_epoll_fd = epoll_create1(::EPOLL_CLOEXEC);
    assert(m_epoll_fd > 0);
    m_active_events = new epoll_event[m_active_event_num];
    assert(m_active_events != nullptr);
}

CEpoll::~CEpoll()
{
    assert(m_active_events != nullptr);
    if(nullptr != m_active_events)
        delete []m_active_events;
    m_active_event_num = 0;
}

int CEpoll::add_event(int fd, uint32_t mask, std::shared_ptr<coro_call_t> &coro, uint32_t time_out)
{
    assert(fd > 0);
    assert(mask != 0);
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof ev);
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    auto pos = m_event_map.find(fd);
    if(m_event_map.end() == pos) {

        auto new_event = std::make_shared<event_t>(fd, mask, time_out, coro);

        if (mask & EV_READ)
            ev.events |= EPOLLIN;
        if (mask & EV_WRITE)
            ev.events |= EPOLLOUT;
        if (mask & EV_ONESHOT)
            ev.events |= EPOLLONESHOT;
        ev.data.ptr = static_cast<void*>(new_event.get());

        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == 0) {
            m_event_map[fd] = new_event;
            m_event_set.insert(new_event);
            return 0;
        } else {
            return -1;
        }
    } else {
        assert(pos->second->get_coro() == coro);
        int new_mask = pos->second->get_mask() | mask;
        assert(!(new_mask & EV_ONESHOT));
        if (new_mask & EV_READ)
            ev.events |= EPOLLIN;
        if (new_mask & EV_WRITE)
            ev.events |= EPOLLOUT;
        if (new_mask & EV_ONESHOT)
            ev.events |= EPOLLONESHOT;
        ev.data.ptr = static_cast<void*>(pos->second.get());
        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == 0) {
            pos->second->set_mask(new_mask);
            return 0;
        } else {
            return -1;
        }
    }
}

int CEpoll::del_event(int fd, uint32_t mask)
{
    assert(fd > 0);
    assert(mask != 0);
    auto pos = m_event_map.find(fd);
    assert(pos != m_event_map.end());
    if(pos != m_event_map.end()) {
        uint32_t new_mask = pos->second->get_mask() & ~mask;
        assert(!(new_mask & EV_ONESHOT));
        struct epoll_event ev;
        std::memset(&ev, 0, sizeof ev);
        if (new_mask & EV_READ)
            ev.events |= EPOLLIN;
        if (new_mask & EV_WRITE)
            ev.events |= EPOLLOUT;
        ev.data.ptr = static_cast<void*>(pos->second.get());
        if (new_mask != EV_NONE) {
            if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == 0) {
                pos->second->set_mask(new_mask);
                return 0;
            } else {
                return -1;
            }
        } else {
            /* Note, Kernel < 2.6.9 requires a non null event pointer even for
             * EPOLL_CTL_DEL. */
            if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &ev) == 0) {
                m_event_set.erase(pos->second);
                m_event_map.erase(pos);
            } else {
                return -1;
            }
        }
    }
    return 0;
}

void CEpoll::poll(coro_yield_t &yield)
{
    while(true) {
        int numevents = epoll_wait(m_epoll_fd, m_active_events, m_active_event_num, 100000);
        std::cout << "numevents : " << numevents << std::endl;
        std::map<uint32_t, std::shared_ptr<event_t>> active_event_map;
        if (numevents > 0) {
            for (int index = 0; index < numevents; ++index) {
                uint32_t mask = 0;
                struct epoll_event *event = m_active_events + index;
                event_t *pactive_event = static_cast<event_t *>(event->data.ptr);

                if (event->events & EPOLLIN) mask |= EV_READ;
                if (event->events & (EPOLLOUT | EPOLLERR | EPOLLHUP)) mask |= EV_WRITE;

                if(mask != EV_NONE) {
                    active_event_map[pactive_event->get_fd()] = m_event_map.find(pactive_event->get_fd())->second;
                }
                if (pactive_event->get_mask() & EV_ONESHOT) {
                    std::cout << pactive_event->get_fd() << " : EV_ONESHOT" << std::endl;
                    auto pos = m_event_map.find(pactive_event->get_fd());
                    m_event_set.erase(pos->second);
                    m_event_map.erase(pos);
                }
            }
        }
        int64_t time_now = current_time();
        for(auto &it : m_event_set) {
            std::cout << "fd : " << it->get_fd() << "   count : " << it.use_count() << std::endl;
            if (time_now < it->get_time_out())
                break;
            if (0 != it->get_time_out()) {
                if (active_event_map.end() != active_event_map.find(it->get_fd())) {
                    it->set_ready_mask(it->get_ready_mask() | EV_TIMEOUT);
                } else {
                    it->set_ready_mask(EV_TIMEOUT);
                    active_event_map[it->get_fd()] = it;
                }
            }
        }
        for(auto &it : active_event_map) {
            yield((*it.second->get_coro().get()), it.second->get_ready_mask());
        }
    }
}

void CEpoll::run() {
    for(auto &it : m_event_set) {
        std::cout << "fd : " << it->get_fd() << "   count : " << it.use_count() << std::endl;
    }
    coro_call_t call(std::bind(&CEpoll::poll, this, std::placeholders::_1));
    call(0);
}

int64_t CEpoll::current_time() const {
    timeval time_now;
    gettimeofday(&time_now, nullptr);
    return (time_now.tv_sec * 1000 + time_now.tv_usec);
}
