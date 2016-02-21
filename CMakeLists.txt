cmake_minimum_required(VERSION 3.3)
project(boost_test1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/main.cpp  src/epoll.h src/epoll.cpp src/co_socket.h src/co_socket.cpp)
add_executable(boost_test1 ${SOURCE_FILES})
target_link_libraries(boost_test1 boost_system boost_context boost_coroutine)