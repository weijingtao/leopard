//
// Created by weitao on 16-2-11.
//

#include <sys/socket.h>
#include <iostream>
#include "co_socket.h"
#include "coroutine.hpp"
#include "epoll.h"

void co_enable_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

ssize_t co_read(int fd, char *buffer, size_t len, coro_yield_t &yield) {
    co_enable_nonblock(fd);
    ssize_t read_bytes = 0;
    while (true){
        read_bytes = read(fd, buffer, len);
        if (0 <= read_bytes)
            break;
        else {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "read yield " << std::endl;
                yield(CEpoll::instance(), 0);
                auto current_event = yield.get();
                if (current_event & EV_TIMEOUT) {
                    errno = ETIMEDOUT;
                    break;
                }
                std::cout << "read ok " << std::endl;
            }
            else
                break;
        }
    }
    return read_bytes;
}

ssize_t co_write(int fd, const char *buffer, size_t len, coro_yield_t &yield) {
    co_enable_nonblock(fd);
    size_t left_bytes   = len;
    ssize_t write_bytes = 0;
    const char *p = buffer;
    while (left_bytes > 0) {
        write_bytes = write(fd, p, left_bytes);
        if (write_bytes <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                auto event = yield.get();
                yield(CEpoll::instance(), 0);
                /*auto current_event = yield.get();
                if (current_event->ready_mask & EV_TIMEOUT) {
                    errno = ETIMEDOUT;
                    break;
                }*/
                continue;
            }
            else if (errno == EINTR)
                continue;
            else
                break;
        }
        left_bytes -= write_bytes;
        p += write_bytes;
    }
    return left_bytes < 0 ? -1 : write_bytes;
}

int co_accept(int fd, struct sockaddr *cli_addr, socklen_t *addr_len) {
    co_enable_nonblock(fd);
    int new_fd;
    std::shared_ptr<coro_call_t> call = std::make_shared<coro_call_t>([&](coro_yield_t &yield){
        while (true) {
            new_fd = accept(fd, cli_addr, addr_len);
            if (new_fd > 0)
                break;
            else {
                if (errno == EINTR)
                    continue;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::cout << "accept yield----" << std::endl;
                    /*std::shared_ptr<CEvent> event = std::make_shared<CEvent>(fd, EV_READ, call);
                    yield(CEpoll::instance(), event);*/
                }
                else
                    break;
            }
        }
    });
//    (*call)();
    return new_fd;
}

int co_connect(int fd, const struct sockaddr *addr, socklen_t addr_len) {
    co_enable_nonblock(fd);
    int result = 0;
    std::shared_ptr<coro_call_t> call = std::make_shared<coro_call_t>([&](coro_yield_t &yield) {
        int error = 0;
        socklen_t error_len = sizeof(error);

        if (connect(fd, addr, addr_len) == -1) {
            if (errno != EINPROGRESS) {
                result = -1;
                return;
            }
            /*std::shared_ptr<CEvent> event = std::make_shared<CEvent>(fd, EV_WRITE, call);
//            CEpoll::get_mutable_instance().add_event(fd, event);
            yield(CEpoll::instance(), event);
//            CEpoll::get_mutable_instance().del_event(fd, event);
            auto current_event = yield.get();
            if (current_event->ready_mask & EV_TIMEOUT) {
                errno = ETIMEDOUT;
            }
            else if (current_event->ready_mask & EV_WRITE) {
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_len) == 0 &&
                    error == 0)
                    result = 0;
            }*/
        }
    });
    return result;
}
