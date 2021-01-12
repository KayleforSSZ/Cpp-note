/*************************************************************
*循环数组实现的阻塞队列，m_back = (m_back + 1) % m_max_size;
*线程安全，每个操作前都要先加互斥锁，操作完后，再解锁
**************************************************************/
#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/mylock.h"
using namespace std;

template <class T>
class block_queue
{
private:
    mylock m_mutex;
    cond m_cond;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;

public:
    // 初始化私有成员
    block_queue(int max_size = 1000)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        // 构造函数创建循环数组
        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    void clear()
    {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    ~block_queue()
    {
        m_mutex.lock();
        if (m_array != nullptr)
            delete[] m_array;
        m_mutex.unlock();
    }

    // 判断队列是否满了
    bool full()
    {
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    // 判断队列是否为空
    bool empty()
    {
        m_mutex.lock();
        if (m_size == 0)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    // 返回队列首元素
    bool front(T &value)
    {
        m_mutex.lock();
        if (m_size == 0)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    // 返回队尾元素
    bool back(T &value)
    {
        m_mutex.lock();
        if (m_size == 0)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    int size()
    {
        int temp = 0;
        m_mutex.unlock();
        temp = m_size;
        m_mutex.unlock();
        return temp;
    }

    int max_size()
    {
        int temp = 0;
        m_mutex.unlock();
        temp = m_max_size;
        m_mutex.unlock();
        return temp;
    }

    // 往队列里添加元素，需要将所有使用队列的线程先唤醒
    // 当有元素 push 进队列，相当于生产者生产了一个元素
    // 若当前没有线程等待条件变量，则唤醒无意义
    bool push(const T &item)
    {
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }

        // 将新增数据放在循环数组的对应位置
        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        m_size++;

        m_cond.broadcast();
        m_mutex.unlock();

        return true;
    }

    // pop 时，如果当前队列没有元素，将会等待条件变量
    bool pop(T &item)
    {
        m_mutex.lock();

        // 多个消费者的时候，这里是用 while 而不是 if
        while (m_size <= 0)
        {
            // 当重新抢到互斥锁，pthread_cond_wait 返回为 0
            if (!m_cond.wait(m_mutex.get()))
            {
                m_mutex.unlock();
                return false;
            }
        }

        // 取出队首元素，这里需要理解一下，使用循环数组模拟队列
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

    // 增加了超时处理
    bool pop(T &item, int ms_timeout)
    {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, nullptr);

        m_mutex.lock();

        // 多个消费者的时候，这里是用 while 而不是 if
        if (m_size <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            // 当重新抢到互斥锁，pthread_cond_wait 返回为 0
            if (!m_cond.timewait(m_mutex.get(), t))
            {
                m_mutex.unlock();
                return false;
            }
        }
        if (m_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }
        // 取出队首元素，这里需要理解一下，使用循环数组模拟队列
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }
};

#endif