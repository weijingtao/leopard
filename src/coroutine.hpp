//
// Created by weitao on 16-2-11.
//

#ifndef BOOST_TEST1_COROUTINE_HPP
#define BOOST_TEST1_COROUTINE_HPP

#include <cstdint>
#include <memory>
#include <functional>
#include <boost/coroutine/all.hpp>

using coro_yield_t = boost::coroutines::symmetric_coroutine<uint32_t>::yield_type;
using coro_call_t  = boost::coroutines::symmetric_coroutine<uint32_t>::call_type;


#endif //BOOST_TEST1_COROUTINE_HPP
