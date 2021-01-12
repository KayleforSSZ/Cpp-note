#ifndef MYLOCK_H
#define MYLOCK_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/* 信号量 */
class sem
{
private:
    sem_t m_sem;

public:
    // 初始化一个信号量
    sem() {
        if(sem_init(&m_sem, 0, 0) != 0) {
            throw std::exception();
        }
    }

    sem(int num) {
        if(sem_init(&m_sem, 0, num) != 0) {
            throw std::exception();
        }
    }
    // 释放一个信号量
    ~sem() {
        sem_destroy(&m_sem);
    }
    // P操作，给信号值+1，通过调用sem_post实现
    bool post() {
        return sem_post(&m_sem) == 0;
    }
    // V操作，给信号值-1，通过调用sem_wait实现
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }
};

/* 互斥锁 */
class mylock
{
private:
    pthread_mutex_t m_mutext;

public:
    // 初始化一把锁
    mylock() {
        if(pthread_mutex_init(&m_mutext, NULL) != 0) {
            throw std::exception();
        }
    }
    // 销毁锁
    ~mylock() {
        pthread_mutex_destroy(&m_mutext);
    }
    // 加锁
    bool lock() {
        return pthread_mutex_lock(&m_mutext) == 0;
    }
    //解锁
    bool unlock() {
        return pthread_mutex_unlock(&m_mutext) == 0;
    }
    // 获得锁
    pthread_mutex_t *get() {
        return &m_mutext;
    }
};

/* 条件变量 */
class cond
{
private:
    pthread_cond_t m_cond;
public:
    // 初始化条件变量
    cond() {
        if(pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception();
        }
    }
    // 销毁条件变量
    ~cond() {
        pthread_cond_destroy(&m_cond);
    }
    // 阻塞等待条件变量
    bool wait(pthread_mutex_t *mutex) {
        int ret = pthread_cond_wait(&m_cond, mutex);
        return ret == 0;
    }
    // 限时阻塞等待条件变量
    bool timewait(pthread_mutex_t *mutex, struct timespec t) {
        int ret = pthread_cond_timedwait(&m_cond, mutex, &t);
        return ret == 0;
    }
    // 唤醒阻塞在条件变量上的一个线程
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }
    // 唤醒阻塞在条件变量上的所有线程
    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
};

#endif







