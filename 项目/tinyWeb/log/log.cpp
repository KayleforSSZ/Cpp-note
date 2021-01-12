#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "log.h"
#include <pthread.h>

using namespace std;
/*
struct tm
{ 　
    int tm_sec;		 /* 秒–取值区间为[0,59]
    int tm_min; 		 /* 分 - 取值区间为[0,59]
    int tm_hour; 	         /* 时 - 取值区间为[0,23]
    int tm_mday;		 /* 一个月中的日期 - 取值区间为[1,31]
    int tm_mon;		 /* 月份（从一月开始，0代表一月） - 取值区间为[0,11]
    int tm_year; 	         /* 年份，其值从1900开始
    int tm_wday; 	         /* 星期–取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推
    int tm_yday; 	         /* 从每年的1月1日开始的天数–取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推
    int tm_isdst; 	         /* 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负
    long int tm_gmtoff;	 /*指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数
    const char *tm_zone;     /*当前时区的名字(与环境变量TZ有关)
}
*/

Log::Log()
{
    m_count = 0;
    m_is_async = false;
}

Log::~Log()
{
    if (m_fp != NULL)
    {
        fclose(m_fp);
    }
}

// 异步需要设置阻塞队列的长度，同步不需要设置
bool Log::init(const char *file_name, int log_buf_size, int split_lines, int max_queue_size)
{
    // 如果设置了max_queue_size，则设置为异步
    if (max_queue_size > 0)
    {
        m_is_async = true;
        // 创建并设置阻塞队列长度
        m_log_queue = new block_queue<string>(max_queue_size);
        pthread_t tid;

        // flush_log_thread 为回调函数，这里表示创建线程异步写日志，回调函数会执行异步写日志函数
        pthread_create(&tid, NULL, flush_log_thread, NULL);
    }

    // 输出内容的长度
    m_log_buf_size = log_buf_size;

    m_buf = new char[m_log_buf_size];
    memset(m_buf, '\0', m_log_buf_size);

    // 日志的最大行度
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    // 从后往前找到第一个 / 的位置
    const char *p = strrchr(file_name, '/');
    char log_full_name[256] = {0};

    // 相当于自定义日志名
    // 若输出的文件名没有 / ，则直接将时间 + 文件名 作为日志名
    if (p == NULL)
    {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    else
    {
        // 将 / 的位置向后移动一个位置，然后复制到 log_name 中
        // p - file_name + 1 是文件所在路径文件夹的长度
        // dirname 相当于 ./
        strcpy(log_name, p + 1);
        strncpy(dir_name, file_name, p - file_name + 1);

        // 后面的参数跟format有关
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }

    m_today = my_tm.tm_mday;

    m_fp = fopen(log_full_name, "a");
    if (m_fp == NULL)
    {
        return false;
    }

    return true;
}

void Log::write_log(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;

    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};

    // 日志分级
    switch (level)
    {
    case 0:
        strcpy(s, "[debug]:"); // 调试代码时的输出
        break;
    case 1:
        strcpy(s, "[info]:"); // 报告系统当前状态
        break;
    case 2:
        strcpy(s, "[warn]:"); // 调试代码时的警告
        break;
    case 3:
        strcpy(s, "[error]:"); // 输出系统错误信息
        break;
    default:
        strcpy(s, "[info]:");
        break;
    }

    m_mutext.lock();

    //  更新现有行数
    m_count++;

    // 日志不是今天或者写入的日志行数是最大行的倍数
    // m_split_lines 为最大行数
    if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0)
    {
        char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0}; // 时间部分

        // 格式化日志名中的时间部分
        snprintf(tail, 16, "%d_%02d_%02d", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        // 如果是时间不是今天，则创建今天的日志，更新 m_today 和 m_count
        if (m_today != my_tm.tm_mday)
        {
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            m_today = my_tm.tm_mday;
            m_count = 0;
        }
        else
        {
            // 超过了最大行，在之前的日志名基础上加后缀，m_count / m_split_lines
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_count / m_split_lines);
        }
        m_fp = fopen(new_log, "a");
    }

    m_mutext.unlock();

    va_list valst;

    // 将传入的 format 参数赋值给 valst，便于格式化输出
    va_start(valst, format);

    string log_str;
    m_mutext.lock();

    // 写入内容格式：时间 + 内容
    // 时间格式化，snprintf 成功返回写字符的总数，其中不包括结尾的 null 字符
    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

    // 内容格式化，用于向字符串中打印数据、数据格式用户自定义，返回写入到字符数组 str 中的字符个数
    int m = vsnprintf(m_buf + n, m_log_buf_size - 1, format, valst);
    m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';

    log_str = m_buf;

    m_mutext.unlock();

    // 若 m_is_async 为 true 表示异步，默认为同步
    // 若异步，则将日志信息加入阻塞队列，同步则加锁向文件中写
    if (m_is_async && !m_log_queue->full())
    {
        m_log_queue->push(log_str);
    }
    else
    {
        m_mutext.lock();
        fputs(log_str.c_str(), m_fp);
        m_mutext.unlock();
    }

    va_end(valst);
}

void Log::flush(void)
{
    m_mutext.lock();
    // 强制刷新写入流缓冲区
    fflush(m_fp);
    m_mutext.unlock();
}