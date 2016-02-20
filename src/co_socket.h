//
// Created by weitao on 16-2-11.
//

#ifndef BOOST_TEST1_SOCKET_H
#define BOOST_TEST1_SOCKET_H

#include <sys/types.h>
#include "coroutine.hpp"

void co_enable_nonblock(int fd);

ssize_t co_read(int fd, char *buffer, size_t len, coro_yield_t &yield);

ssize_t co_write(int fd, const char *buffer, size_t len, coro_yield_t &yield);

int co_accept(int fd, struct sockaddr *cli_addr, socklen_t *addr_len);

int co_connect(int fd, const struct sockaddr *addr, socklen_t addr_len);


#endif //BOOST_TEST1_SOCKET_H
