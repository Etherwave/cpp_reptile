#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <iostream>

using namespace std;

#ifndef AMAZING_THREADPOOL_H
#define AMAZING_THREADPOOL_H

class ThreadPool {

public:
    ThreadPool(size_t);                          //构造函数
    // 使用template关键字定义了两个新的类别，F和一个可变模版参数 Args 其声明方式就是 <class... T>,这里是<class... Args>
    // result_of 是推导F(Args...)这个函数的返回值
    // std::future对象来获取异步操作的结果
    // 整体这句话的意思就是定义了一个参数是Args的函数F，
    // 定义了一个函数enqueue，其参数是一个函数的地址和其参数，参数的类别和个数不定，里边的&&是
    // &&是右值引用（即将消亡的值就是右值，函数返回的临时变量也是右值）
    // &可以绑定左值（左值是指表达式结束后依然存在的持久对象）
    // 这里的&&是为了进行右值引用
    // 移动构造函数： 直接用参数对象里面的指针来初始化当前对象的指针，是一种浅层复制。
    // 做完这种指针对指针的复制，即把参数指针所指向的对象转给了当前正在被构造的这个指针，函数体内接着就把参数n里面的指针给置为空指针， 这个对象里面的指针置为空指针，将来析构函数去析构它的时候是delete一个空指针，就不会发生多次析构的事情，这个就是一个移动构造函数。
    // 该enqueue函数的返回值是一个异步调用的结果，其类型是F(Args...)的返回类型
    template<class F, class... Args>             //类模板
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;//任务入队
    ~ThreadPool();                              //析构函数

private:
    std::vector< std::thread > workers;            //线程队列，每个元素为一个Thread对象
    // 这里的function<void()>就是说一个返回值是void的函数的意思
    // 为什么不直接用void*的函数指针呢？是因为function还包含了函数的调用类型等信息，在模板编程时是安全的，emmmm，大致是这么个意思
    std::queue< std::function<void()> > tasks;     //任务队列，每个元素为一个函数对象

    std::mutex queue_mutex;                        //互斥量
    std::condition_variable condition;             //条件变量
    bool stop;                                     //停止
};

// 构造函数，把线程插入线程队列，插入时调用emplace_back()，用匿名函数lambda初始化Thread对象
inline ThreadPool::ThreadPool(size_t threads) : stop(false){
    // 这里用emplace_back感觉是想避免复制构造或移动构造，比较线程的创建比较耗时，经过我测试移动构造函数，我认为是在emplace_back中传
    // 类的构造函数所需要的参数，这样才可以避免复制构造或移动构造

    std::function<void()> f=[this]
    {
        for(;;)
        {
            // task是一个函数类型，从任务队列接收任务
            std::function<void()> task;
            {
                //给互斥量加锁，锁对象生命周期结束后自动解锁
                std::unique_lock<std::mutex> lock(this->queue_mutex);

                //（1）当匿名函数返回false时才阻塞线程，阻塞时自动释放锁。
                //（2）当匿名函数返回true且受到通知时解阻塞，然后加锁。
                // this->stop == 1 说明线程池要停止了，该线程应该结束
                // !this->tasks.empty() 说明有了新的任务，那就派出一个线程去运行这个函数
                this->condition.wait(lock,[this]{ return this->stop || !this->tasks.empty(); });

                if(this->stop && this->tasks.empty())
                    return;

                //从任务队列取出一个任务
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }                            // 自动解锁
            task();                      // 执行这个任务
        }
    };
    for(size_t i = 0; i<threads; ++i)
        workers.emplace_back(f);
}

// 添加新的任务到任务队列
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    // 获取函数返回值类型
    using return_type = typename std::result_of<F(Args...)>::type;

    // 创建一个指向任务的只能指针
    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);  //加锁
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });          //把任务加入队列
    }                                                   //自动解锁
    condition.notify_one();                             //通知条   件变量，唤醒一个线程
    return res;
}

// 析构函数，删除所有线程
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

#endif //AMAZING_THREADPOOL_H
