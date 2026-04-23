#include<goruntime/runtime.h>
#include<windows.h>
namespace goruntime{
    Runtime::Runtime(std::size_t worker_count){
        if(worker_count==0){
            worker_count=std::thread::hardware_concurrency();
            if(worker_count==0){
                worker_count=4;
            }
        }
        workers_.reserve(worker_count);
        for(std::size_t i=0;i<worker_count;++i){
            workers_.emplace_back([this]{
                worker_loop();
            });
        }
    }
    Runtime::~Runtime(){
        shutdown();
    }
    void Runtime::submit(Task task){
        if(!task){
            throw std::invalid_argument("Task cannot be empty");
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if(stooping_.load(std::memory_order_acquire)){
                throw std::runtime_error("Runtime is stopping, cannot submit new tasks");
            }
            task_queue_.push(std::move(task));
        }
        cv_.notify_one();
    }
    void Runtime::shutdown(){
        bool expected=false;
        if(!stooping_.compare_exchange_strong(expected,true,std::memory_order_acq_rel)){
            return;
        }
        cv_.notify_all();
        for(auto& worker:workers_){
            if(worker.joinable()){
                worker.join();
            }
        }
    }
    std::size_t Runtime::worker_count() const noexcept{
        return workers_.size();
    }
    std::size_t Runtime::pending_task() const{
        std::lock_guard<std::mutex> lock(mutex_);
        return task_queue_.size();
    }
    void Runtime::worker_loop(){
        while(true){
            Task task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock,[this]{
                    return stooping_.load(std::memory_order_acquire) || !task_queue_.empty();
                });
                if(stooping_.load(std::memory_order_acquire) && task_queue_.empty()){
                    return;
                }
                task=std::move(task_queue_.front());
                task_queue_.pop();
            }
            try{
                task();
            }catch(...){
                
            }

        }
    }
}