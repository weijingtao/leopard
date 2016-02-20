//
// Created by weitao on 16-2-11.
//

#ifndef BOOST_TEST1_EVENT_H
#define BOOST_TEST1_EVENT_H

#include <cstdint>
#include <memory>
#include <iostream>
#include <boost/noncopyable.hpp>

enum
{
    EV_NONE      = 0X00,
    EV_READ      = 0X01,
    EV_WRITE     = 0X02,
    EV_TIMEOUT   = 0X04,
    EV_CLOSE     = 0X08,
    EV_EXCEPTION = 0X10,
    EV_SIGNAL    = 0X20,
    EV_ONESHOT   = 0X40
};

template <typename coro_type>
class CEvent : public boost::noncopyable
{
public:
    CEvent(int fd, uint32_t mask, uint32_t time_out, std::shared_ptr<coro_type> &coro)
            : m_fd(fd),
              m_mask(mask),
              m_ready_mask(0),
              m_time_out(time_out),
              m_coro(coro) {
        std::cout << "CEvent fd : " << m_fd << "  count : " << m_coro.use_count() << std::endl;
    }

    ~CEvent() {
        std::cout << "~CEvent fd : " << m_fd << "  count : " << m_coro.use_count() << std::endl;
    }

    bool operator<(const CEvent &rhs) {
        if(this->m_time_out == rhs.m_time_out)
            return this < &rhs;
        return this->m_time_out < rhs.m_time_out;
    }

    bool operator++(int fd) {
        return m_fd == fd;
    }

    void set_fd(int fd) { m_fd = fd; }

    int get_fd() const { return m_fd; }

    void set_mask(uint32_t mask) { m_mask = mask; }

    uint32_t get_mask() const { return m_mask; }

    void set_ready_mask(uint32_t ready_mask) { m_ready_mask = ready_mask; }

    uint32_t get_ready_mask() const { return m_ready_mask; }

    void set_time_out(uint32_t time_out) { m_time_out = time_out; }

    uint32_t get_time_out() const { return m_time_out; }

    void set_coro(std::shared_ptr<coro_type> &coro) { m_coro = coro; }

    std::shared_ptr<coro_type>& get_coro() { return m_coro; }


private:
    int      m_fd;
    uint32_t m_mask;
    uint32_t m_ready_mask;
    uint32_t m_time_out;
    std::shared_ptr<coro_type> m_coro;
};

#endif //BOOST_TEST1_EVENT_H
