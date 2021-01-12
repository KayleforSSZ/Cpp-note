#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

class Log
{
private:
    char dir_name[128];               // 路径名，比如 /xx/xxx/xxx.log 里的 /xx/xxx/
    char log_name[128];               // log 文件名，比如 xxx.log
    int m_split_lines;                // 日志最大行数
    int m_log_buf_size;               // 日志缓冲区大小
    long long m_count;                // 日志行数记录
    int m_today;                      // 按天分文件，记录当前时间是哪一天
    FILE *m_fp;                       // 打开 log 的文件指针
    char *m_buf;                      // 要输出的内容
    block_queue<string> *m_log_queue; // 阻塞队列
    bool m_is_async;                  // 是否异步标志位
    mylock m_mutext;                  // 锁

public:
    // C++11之后使用局部变量懒汉模式不用加锁
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    // 实现日志创建、写入方式的判断 可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志队列
    bool init(const char *file_name, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    // 异步写日志公有方法，调用私有方法 async_write_log
    static void *flush_log_thread(void *args)
    {
        Log::get_instance()->async_write_log();
    }

    // 写入日志文件中的具体内容，主要实现日志分级、分文件、格式化输出内容
    void write_log(int level, const char *format, ...);

    // 强制刷新缓冲区
    void flush(void);

private:
    Log();
    virtual ~Log();

    // 异步写日志方法
    void *async_write_log()
    {
        string single_log;

        // 从阻塞队列中取出一条日志内容，写入文件，注意如果阻塞队列上没有日志内容，在pop里会一直等待
        while (m_log_queue->pop(single_log))
        {
            m_mutext.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutext.unlock();
        }
    }
};

// 这四个宏在其他文件中使用，主要用于不同类型的日志输出
#define LOG_DEBUG(format, ...) Log::get_instance()->write_log(0, format, ##__VA_ARGS__);
#define LOG_INFO(format, ...) Log::get_instance()->write_log(1, format, ##__VA_ARGS__);
#define LOG_WARN(format, ...) Log::get_instance()->write_log(2, format, ##__VA_ARGS__);
#define LOG_ERROR(format, ...) Log::get_instance()->write_log(3, format, ##__VA_ARGS__);

#endif