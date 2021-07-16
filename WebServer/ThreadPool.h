//
// Created by sky on 2021/7/8.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <functional>
#include <memory>
class ThreadPool {
public:
    ThreadPool(size_t threadNum=8): pool_(std::make_shared<Pool>()){
        assert(threadNum>0);
        for(size_t i=0;i<threadNum;++i){
            std::thread(
                    [pool = pool_]{
                        //上锁,析构函数自动解锁
                        std::unique_lock<std::mutex> locker(pool->mtx);
                        while(true){
                            if(!pool->tasks.empty()){
                                auto task = std::move(pool->tasks.front());
                                pool->mtx.unlock();
                                //解锁执行任务,执行完再上锁
                                task();
                                pool->mtx.lock();
                            }else if(pool->isClosed)    break;
                            //任务列表为空，等待连接启用线程
                            else    pool->cond.wait();
                        }
                    }
                    ).detach();
        }
    }
    ThreadPool() = default;
    //移动构造函数
    ThreadPool(ThreadPool&&) = default;
    ~ThreadPool(){
        if(static_cast<bool>(pool_)){
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class T>
    void AddTask(T&& task){
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.template emplace(std::forward<T>(task));
        }
        //条件变量解除
        pool_->cond.notify_one();
    }
private:
    struct Pool{
        std::mutex mtx;
        std::condition_variable cond;
        std::queue<std::function<void()>> tasks;
        bool isClosed;
    };
    std::shared_ptr<Pool> pool_;
};


#endif //WEBSERVER_THREADPOOL_H
