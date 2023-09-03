#include "storage_pthread_pool.h"

PthreadPool* PthreadPool::thread_pool = nullptr;

PthreadPool* PthreadPool::GetInstance(){
    if(!thread_pool){
        thread_pool = new PthreadPool(4);
    }
    return thread_pool;
}

void PthreadPool::DestroyInstance(){
    if(thread_pool) {
        delete thread_pool;
        thread_pool = nullptr;
    }
}

PthreadPool::PthreadPool(int posix_size) : pool_(std::make_shared<Pool>()){
    for(size_t i = 0; i < posix_size; i++) {
        std::thread([pool = pool_] {
        std::unique_lock<std::mutex> locker(pool->mtx);
        while(true) {
            if(!pool->tasks.empty()) {
                auto task = std::move(pool->tasks.front());
                pool->tasks.pop();
                locker.unlock();
                task();
                locker.lock();
            } 
            else if(pool->isClose){
                break;
            }
            else {
                pool->cond.wait(locker);
            }
        }
        }).detach();
    }
}

PthreadPool::~PthreadPool(){
    if(static_cast<bool>(pool_)){
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->isClose = true;
        }
        //唤醒所有等待的线程,他们会自行退出，系统内核做善后
        pool_->cond.notify_all();
    }
}

