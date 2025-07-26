#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unistd.h>
#include <sys/syscall.h>
#include <future>

class ThreadPool
{
private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::vector<std::thread> m_threads;             // 容器中存放线程对象
    std::queue<std::function<void()>> m_task_queue; // 函数对象的队列
    std::atomic_bool m_flag_stop;                   // 析构函数中，将该标志设置为 true 表示停止所有线程
    std::string m_type;                             // 线程类型，IO 或 work
public:
    ThreadPool(size_t thread_num, const std::string& type);
    ~ThreadPool();

    void add_task(std::function<void ()> fn);
    void thread_info();
    ssize_t size();

    void stop();
};

