
#include "ThreadPool.h"
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>

ThreadPool::ThreadPool(int thread_num, const std::string& type) : m_thread_num(thread_num), m_type(type), m_stop_flag(false)
{
    for (size_t i = 0; i < m_thread_num; i++)
    {
        m_threads.emplace_back(std::thread([&]
        {
            printf("create %s thread id %d .\n", m_type.c_str(), syscall(SYS_gettid));
            while (m_stop_flag == false)
            {
                std::function<void ()> task;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cv.wait(lock, [&]{ return m_stop_flag == true || m_task_queue.empty() == false; });
                    if (m_stop_flag == true && m_task_queue.empty())
                        break;
                    
                    task = std::move(m_task_queue.front());
                    m_task_queue.pop(); 
                }
                task();
            }
            printf("%s thread id %d exit.\n", m_type.c_str(), syscall(SYS_gettid));
        }));
    }
}

ThreadPool::~ThreadPool() { stop(); }

void ThreadPool::add_task(std::function<void ()> fn)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task_queue.push(std::move(fn));
    }
    m_cv.notify_one();
}

void ThreadPool::stop()
{
    m_stop_flag = true;
    m_cv.notify_all();
    for (auto & t : m_threads)
    {
        t.join();
    }
    m_threads.clear();
}

int ThreadPool::size() const
{
    return m_threads.size();
}
