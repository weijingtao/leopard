//
// Created by weitao on 16-2-11.
//

#ifndef BOOST_TEST1_TEST_WORKER_H
#define BOOST_TEST1_TEST_WORKER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <functional>
#include <iostream>
#include "epoll.h"
#include "coroutine.hpp"
#include "co_socket.h"

class CTestWorker : public std::enable_shared_from_this<CTestWorker>
{
public:
    CTestWorker(int fd, const sockaddr &addr, socklen_t addr_len)
            : m_fd(fd), m_addr_len(addr_len) {
        std::memcpy(&m_addr, &addr, addr_len);
        std::cout << "CTestWorker" << std::endl;
    }

    ~CTestWorker() {
        std::cout << "~CTestWorker" << std::endl;
    }

    void start() {
        std::shared_ptr<coro_call_t> new_coro(new coro_call_t(std::bind(&CTestWorker::do_process, this->shared_from_this(), std::placeholders::_1)));
//        std::shared_ptr<CEvent> event = std::make_shared<CEvent>(m_fd, EV_READ, new_coro);
        CEpoll::instance_().add_event(m_fd, EV_READ, new_coro, 0);
    }

    void do_process(coro_yield_t &yield) {
        while(true) {
            std::cout << "do_process" << std::endl;
            char buffer[1024];
            int nread = co_read(m_fd, buffer, sizeof buffer, yield);
            std::cout << "nread : " << nread << std::endl;
            if(0 == nread) {
                CEpoll::instance_().del_event(m_fd, EV_READ | EV_WRITE);
                close(m_fd);
                yield(CEpoll::instance(), 0);
                return;
            }
            std::cout << "read : " << nread << std::endl;
            buffer[nread] = '\0';
            std::cout << buffer << std::endl;
            co_write(m_fd, buffer, nread, yield);
        }
    }

private:
    int       m_fd;
    sockaddr  m_addr;
    socklen_t m_addr_len;
};

#endif //BOOST_TEST1_TEST_WORKER_H
