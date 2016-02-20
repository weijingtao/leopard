cmake_minimum_required(VERSION 3.3)
project(boost_test1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp main_coroutine.h epoll.h epoll.cpp co_socket.h co_socket.cpp)
add_executable(boost_test1 ${SOURCE_FILES})
target_link_libraries(boost_test1 boost_system boost_context boost_coroutine)