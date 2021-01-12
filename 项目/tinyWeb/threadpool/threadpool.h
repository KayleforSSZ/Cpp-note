#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/mylock.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool
{
private:
    int m_thread_number;         // 线程池内的线程数量
    int m_max_requests;          // 任务请求队列允许的最大请求数
    pthread_t *m_threads;        // 线程数组，大小为m_thread_number
    std::list<T *> m_workqueue;  // 任务请求队列
    mylock m_workqueue_lock;     // 请求队列的锁
    sem m_workqueue_state;       // 请求队列上的信号量，表示任务队列是否为空
    bool m_shutdown;             // 线程池使用状态
    connection_pool *m_connPool; // 数据库连接池指针

private:
    static void *worker(void *arg); // 工作线程运行的函数，不断从工作队列中去除任务并执行，要求为静态函数
    void run();

public:
    threadpool(connection_pool *connPool, int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T *request); // 向请求队列中插入任务请求
};

template <typename T>
threadpool<T>::threadpool(connection_pool *connPool, int thread_number, int max_requests) : m_thread_number(thread_number), m_max_requests(max_requests), m_shutdown(false), m_threads(NULL),m_connPool(connPool)
{
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();

    // 线程数组初始化
    m_threads = new pthread_t[m_thread_number];
    if (m_threads == nullptr)
        throw std::exception();

    int ret;
    for (int i = 0; i < m_thread_number; i++)
    {

        // 循环创建线程，并将工作线程按要求进行运行
        ret = pthread_create(m_threads + i, NULL, worker, this); // 创建线程，worker为回调函数
        if (ret != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        ret = pthread_detach(m_threads[i]); // 设置线程分离，不用单独对线程进行回收
        if (ret != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_shutdown = true;
}

// 向请求队列中添加任务，通过互斥锁保证线程安全，添加完成后通过信号量提醒有无任务要处理
template <typename T>
bool threadpool<T>::append(T *request)
{
    // 上锁
    m_workqueue_lock.lock();

    // 如果请求队列已满，添加失败
    if (m_workqueue.size() > m_max_requests)
    {
        m_workqueue_lock.unlock();
        return false;
    }

    // 添加任务
    m_workqueue.push_back(request);
    m_workqueue_lock.unlock();

    // 信号量提醒有任务要处理
    m_workqueue_state.post();

    return true;
}

// 线程处理函数
template <typename T>
void *threadpool<T>::worker(void *arg)
{
    // 将参数转化为线程池类，调用成员方法
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

// 工作线程从任务队列中去除某个任务进行处理，注意线程同步
template <typename T>
void threadpool<T>::run()
{
    while (!m_shutdown)
    {
        // 信号量等待
        m_workqueue_state.wait();

        // 被唤醒后先加互斥锁
        m_workqueue_lock.lock();
		
        if (m_workqueue.empty())
        { // 如果队列中没有任务，那么解锁后继续循环阻塞
            m_workqueue_lock.unlock();
            continue;
        }

        // 从任务队列中取出一个任务，并从请求队列中删除
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_workqueue_lock.unlock();
        if (!request)
            continue;

        // 从连接池中取出一个数据库连接
        // request->mysql = m_connPoll->GetConnection();
        connectionRAII mysqlcon(&request->mysql, m_connPool);
        // process（模板类中的方法，这里是http类）进行处理
        request->process();

        // 将数据库连接放回连接池
        // m_connPoll->ReleaseConnection(request->mysql);
    }
}

#endif