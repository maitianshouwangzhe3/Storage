#ifndef __PTHREAD_POOL__
#define __PTHREAD_POOL__

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

struct Pool{
    std::mutex mtx;                             //互斥锁
    std::condition_variable cond;               //信号量
    bool isClose;                               //是否关闭
    std::queue<std::function<void()>> tasks;    //任务队列
};



class PthreadPool{
private:
    PthreadPool() = default;
    PthreadPool(PthreadPool&&) = default;
    PthreadPool(int posix_size = 4);
    ~PthreadPool();
public:
    template<class F>
    void push(F&& task);

    static PthreadPool* GetInstance();
    static void DestroyInstance();

private:
    std::shared_ptr<Pool> pool_;
    static PthreadPool* thread_pool;
};

template<class F>
void PthreadPool::push(F&& task){
    {
        std::lock_guard<std::mutex> locker(pool_->mtx);
        //forward()完美转发，传进来的值是什么形式传递给下一个函数也是什么形式
        pool_->tasks.emplace(std::forward<F>(task));
    }
    //随机唤醒一个等待的线程
    pool_->cond.notify_one();
}


#endif