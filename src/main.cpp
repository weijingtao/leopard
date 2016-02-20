#include <iostream>
#include "acceptor.hpp"
#include "test_worker.hpp"



int main(int argc, char* argv[])
{
    boost::coroutines::symmetric_coroutine<void>::call_type *pcall_1;
    boost::coroutines::symmetric_coroutine<void>::call_type *pcall_2;

    boost::coroutines::symmetric_coroutine<void>::call_type call_1([&](boost::coroutines::symmetric_coroutine<void>::yield_type& yield){
        std::cout << "call_1" << std::endl;
        yield(*pcall_2);
    });

    boost::coroutines::symmetric_coroutine<void>::call_type call_2([&](boost::coroutines::symmetric_coroutine<void>::yield_type &yield){
//        int a = yield.get();
        std::cout << "call_2" << std::endl;
        yield();
    });
    pcall_1 = &call_1;
    pcall_2 = &call_2;
    /*while(true) {
        call_1();
    }*/
    CAcceptor<CTestWorker> acceptor;
    int ret = acceptor.accept("127.0.0.1:50000");
    std::cout << "ret : " << ret << std::endl;
    CEpoll::instance_().run();
    return 0;
}