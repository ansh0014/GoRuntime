#include "goruntime/runtime.h"
namespace goruntime{
    void Runtime::worker_loop(){
        while(true){
            Task task;
            if(!task_queue_.wait_pop(task)){
                return;
            }
            try{
                task();
            }catch(...){
                // Handle task exceptions if needed
            }
        }
    }
}