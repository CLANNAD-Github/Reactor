#pragma once

#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <string>

class ThreadPool
{
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::function<void ()>> m_task_queue;
    std::atomic<bool> m_stop_flag;
    std::vector<std::thread> m_threads;
    int m_thread_num;
    std::string m_type;

public:
    ThreadPool(int thread_num, const std::string& type);
    ~ThreadPool();

    void add_task(std::function<void ()> fn);
    void stop();
    int size() const;
};
