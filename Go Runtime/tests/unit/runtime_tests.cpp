#include<goruntime/runtime.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include<windows.h>
using namespace std::chrono_literals;
namespace{
    int test_executes_submitted_tasks(){
        goruntime::Runtime runtime(4);
        constexpr int kTaskCount=200;
        std::atomic<int>done{0};
        std::mutex m;
        std::condition_variable cv;
        for(int i=0;i<kTaskCount;i++){
            runtime.submit([&]{
            const int value=done.fetch_add(1)+1;
            if(value==kTaskCount){
                std::lock_guard<std::mutex>lock(m);
                cv.notify_one();
            }
        });
        }
        std::unique_lock<std::mutex>lock(m);
        const bool completed =cv.wait_for(lock,3s,[&]{return done.load()==kTaskCount;});
        runtime.shutdown();
        if(!completed){
            std::cerr<<"Test failed: Not all tasks completed within the expected time."<<std::endl;
            return 1;
        }
        return 0;
    }
    int test_submit_rejected_after_shutdown(){
        goruntime::Runtime runtime(2);
        runtime.shutdown();
        try{
            runtime.submit([]{});

        }catch(const std::runtime_error&){
            return 0;
        }catch(...){
            std::cerr<<"test submit rejected after shutdown: Caught unexpected exception type."<<std::endl;
            return 1;
        }
        std::cerr<<"Test failed: Task submission should have been rejected after shutdown."<<std::endl;
        return 1;
    }
}
int main() {
    int failures = 0;
    failures += test_executes_submitted_tasks();
    failures += test_submit_rejected_after_shutdown();

    if (failures == 0) {
        std::cout << "All runtime tests passed\n";
        return 0;
    }

    std::cerr << failures << " test(s) failed\n";
    return 1;
}