#ifndef _SRC_NET_ACCEPTOR_H
#define _SRC_NET_ACCEPTOR_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <boost/noncopyable.hpp>
#include "co_socket.h"
#include "event.hpp"
#include "epoll.h"

template <typename T>
class CAcceptor : public boost::noncopyable
{
public:
    CAcceptor() : m_accept_fd(-1) {

    }

	~CAcceptor() {
        if(m_accept_fd > 0) {
            close(m_accept_fd);
        }
    }

    int accept(const std::string &addr) {
        char ip[64]   = {0};
        uint32_t port = 50000;
//        sscanf(addr.c_str(), "%s:%u", ip, &port);
        sockaddr_in bind_addr;
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port   = htons(port);
        bind_addr.sin_addr.s_addr = INADDR_ANY;
        /*if(1 != inet_pton(AF_INET, ip, &bind_addr.sin_addr.s_addr))
            return -1;*/
        m_accept_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(-1 == m_accept_fd)
            return -2;
        int flags = fcntl(m_accept_fd, F_GETFL, 0);
        fcntl(m_accept_fd, F_SETFL, flags | O_NONBLOCK);
        int op = 1;
        setsockopt(m_accept_fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof op);

        if(0 != bind(m_accept_fd, (sockaddr*)&bind_addr, sizeof bind_addr))
            return -3;
        if(0 != listen(m_accept_fd, 10))
            return -4;

        std::shared_ptr<coro_call_t> new_coro(new coro_call_t(std::bind(&CAcceptor::do_accept,
                                                                        this,
                                                                        std::placeholders::_1)));
//        std::shared_ptr<CEvent> event = std::make_shared<CEvent>(m_accept_fd, EV_READ, new_coro);
        std::cout << "------------------------" << std::endl;
//        std::cout << event->fd << " : " << event.use_count() << std::endl;
        CEpoll::instance_().add_event(m_accept_fd, EV_READ,  new_coro, 0);
//        std::cout << event->fd << " : " << event.use_count() << std::endl;
        std::cout << "------------------------" << std::endl;
        return 0;
    }

    void do_accept(coro_yield_t &yield) {
        std::cout << "do_accept" << std::endl;
        int client_fd;
        while(true) {
            sockaddr client_addr;
            std::memset(&client_addr, 0, sizeof client_addr);
            socklen_t addr_len = sizeof client_addr;
            client_fd = ::accept(m_accept_fd, &client_addr, &addr_len);
            if (client_fd > 0)
            {
                auto new_task = std::make_shared<T>(client_fd, client_addr, addr_len);
                new_task->start();
            }
            else {
                if (errno == EINTR)
                    continue;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::cout << "accept yield" << std::endl;
//                    auto event = yield.get();
                    yield(CEpoll::instance(), 0);
                }
                else
                    break;
            }
        }
    }

	std::shared_ptr<T> create_task(int fd, const sockaddr &addr, socklen_t addr_len) {
        auto *new_task = new T(fd, addr, addr_len);
        return new_task->shared_from_this();
    }

private:
    int m_accept_fd;

};



#endif // _SRC_NET_ACCEPTOR_H
