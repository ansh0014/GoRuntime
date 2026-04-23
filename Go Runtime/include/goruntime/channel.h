#pragma once
#include<condition_variable>
#include<cstddef>
#include<mutex>
#include<deque>
#include<stdexcept>
#include<utility>
namespace goruntime{
    template <typename T>
    class BufferedChannel{
        public:
        explicit BufferedChannel(std::size_t capacity): capacity_(capacity){
            if(capacity_==0){
                throw std::invalid_argument("BufferedChannel capacity must be positive");

            }
        }
        Buffered
    }
}