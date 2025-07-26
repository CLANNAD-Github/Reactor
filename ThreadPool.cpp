#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t thread_num, const std::string& type) : m_flag_stop(false), m_type(type)
{
    for (size_t i = 0; i < thread_num; i++)
    {
        m_threads.emplace_back([this]
        {
            thread_info();
            while (m_flag_stop == false)
            {
                std::function<void ()> task;
                // 使用 {} 构建局部作用域
                {
                    // 创建一个锁，退出 {} 后会自动销毁
                    std::unique_lock<std::mutex> lock(this->m_mutex);
                    this->m_condition.wait(lock, [this]
                    {
                        // printf("waiting function data...\n");
                        return ((this->m_flag_stop == true) || (this->m_task_queue.empty() == false));
                    });

                    // 在线程池停止之前，如果队列中还有任务，执行完再退出
                    if ((this->m_flag_stop == true) && (this->m_task_queue.empty() == true))
                    {
                        // 终止标志被设置，说明线程池中需要停止，检查一下队列中是否还有未出队的任务，如果没有（即：队列为空）说明该线程已经不需要执行任务，直接退出
                        return;
                    }
                    // 流程进入到此处，说明队列中还有任务，说明该线程可以出队任务进行处理
                    task = std::move(this->m_task_queue.front());
                    this->m_task_queue.pop();
                }   // 锁的局部作用域结束，在这里将销毁 lock 对象，并释放锁
                task();
            }
        });
    }
}

void ThreadPool::add_task(std::function<void ()> fn)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task_queue.push(fn);
    }
    m_condition.notify_one();
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::stop()
{
    if (m_flag_stop == true)
        return;
    
    m_flag_stop = true;
    m_condition.notify_all();
    for (auto &th : m_threads)
    {
        th.join();
    }
    printf("~ThreadPool() ... %s\n", m_type.c_str());
}

void ThreadPool::thread_info()
{
    printf("Thread ID(%d), Type(%s)\n", syscall(SYS_gettid), m_type.c_str());
}

ssize_t ThreadPool::size()
{
    return m_threads.size();
}
