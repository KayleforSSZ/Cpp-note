#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "./lock/mylock.h"
#include "./threadpool/threadpool.h"
#include "./timer/lst_timer.h"
#include "./http/http_conn.h"
#include "./log/log.h"
#include "./CGImysql/sql_connection_pool.h"

using namespace std;

#define MAX_FD 65536           //最大文件描述符
#define MAX_EVENT_NUMBER 10000 //最大事件数
#define TIMESLOT 5             //最小超时单位


//#define SYNLOG //同步写日志
// #define ASYNLOG //异步写日志

//#define ET //边缘触发非阻塞
#define LT //水平触发阻塞

//这三个函数在http_conn.cpp中定义，改变链接属性
extern int addfd(int epollfd, int cfd, bool one_shot);
extern int removefd(int epollfd, int cfd);
extern int setnonblocking(int cfd);

//设置定时器相关参数
static int pipefd[2];            // 传递信号的管道
static sort_timer_lst timer_lst; // 定时器容器链表
static int epollfd = 0;          // epoll 树根

// 信号处理函数：信号处理函数中仅仅通过管道发送信号值，不处理信号对应的逻辑，缩短异步执行时间，减少对主程序的影响。
void sig_handler(int sig)
{
    // 为保证函数的可重入性，保留原来的 errno
    // 可重入性表示中断后再次进入该函数，环境变量与之前相同，不会丢失数据
    int save_errno = errno;
    int msg = sig;

    // 将信号值从管道写段写入，传输字符类型，而非整型
    send(pipefd[1], (char *)&msg, 1, 0);

    // 将原来的 errno 赋值给当前的 errno
    errno = save_errno;
}

// 设置信号函数，仅关注 SIGSTERM 和 SIGALRM 两个信号
void addsig(int sig, void(handler)(int), bool restart = true)
{
    // 创建 sigaction 结构体变量
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    // 信号处理函数中仅仅发送信号值，不做对应逻辑处理
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    //将所有信号添加到信号集中
    sigfillset(&sa.sa_mask);

    // 注册 sigaction 函数
    assert(sigaction(sig, &sa, NULL) != -1);
}

// 定时处理任务，重新定时以不断触发 SIGALRM 信号
void timer_handler()
{
    timer_lst.tick();
    alarm(TIMESLOT);
}

// 定时事件，从内核事件表删除事件，关闭文件描述符，释放连接资源
void cb_func(client_data *user_data)
{
    // 删除非活动连接在 socket 上的注册事件
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);

    // 关闭文件描述符
    close(user_data->sockfd);

    // 减少连接数
    http_conn::m_user_count--;

    // LOG_INFO("close fd %d", user_data->sockfd);
    // Log::get_instance()->flush();
}

void show_error(int connfd, const char *info)
{
    printf("%s", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int main(int argc, char *argv[])
{

// #ifdef ASYNLOG
    // Log::get_instance()->init("ServerLog", 2000, 800000, 8); // 异步日志模型
// #endif

// #ifdef SYNLOG
    // Log::get_instance()->init("ServerLog", 2000, 800000, 0); // 同步日志模型
// #endif

    if (argc <= 1)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
	// int port = atoi("9001");
    addsig(SIGPIPE, SIG_IGN);

    // 创建数据库连接池
    connection_pool *connPool = connection_pool::GetInstance();
    connPool->init("localhost", "root", "123456", "web", 3306, 8);
	
	// cout << "create database pool success" << endl;
	
    // 创建线程池
    threadpool<http_conn> *pool = NULL;
    try
    {
        pool = new threadpool<http_conn>(connPool);
    }
    catch (...)
    {
        return 1;
    }
	
	// cout << "create thread pool success" << endl;

    // 创建MAX_FD个http类对象
    http_conn *users = new http_conn[MAX_FD];
    assert(users);
	
	// cout << "create http_conn array success" << endl;

    // 初始化数据库读取表
    users->initmysql_result(connPool);
	
	// cout << "init mysql success" << endl;
	
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);

    // 创建内核事件表
    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    if (epollfd == -1)
    {
        perror("epoll_create error");
        exit(1);
    }

    // 将 listenfd 放到监听红黑树上
    addfd(epollfd, listenfd, false);

    // 将上述 epollfd 赋值给 http 类对象的 m_epollfd 属性
    http_conn::m_epollfd = epollfd;

    // 创建管道套接字
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);

    // 设置管道写端为非阻塞，为什么写端要非阻塞
    setnonblocking(pipefd[1]);
    // 设置管道读端为 ET 非阻塞
    addfd(epollfd, pipefd[0], false);

    // 传递给主循环的信号值，这里只关注 SIGALRM 和 SIGTERM
    addsig(SIGALRM, sig_handler, false);
    addsig(SIGTERM, sig_handler, false);

    // 循环条件
    bool stop_server = false;

    // 创建连接资源数组
    client_data *users_timer = new client_data[MAX_FD];
	// cout << "create users_timer array success" << endl;
    // 超时标志
    bool timeout = false;
    // 每隔 TIMESLOT 时间触发 SIGALRM 信号
    alarm(TIMESLOT);
	
	// cout << "epoll start listen" << endl;
	
    while (!stop_server)
    {
        // 等待所监控的文件描述符上有事件发生
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1); // -1表示阻塞，也可设置为非阻塞轮询
        if (number == -1 && errno != EINTR)
        {

            // LOG_ERROR("%s", "epoll failure");

            break;
        }

        // 对就绪事件进行处理
        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            // 如果是有新的客户端发起连接，即 listenfd 满足读事件
            if (sockfd == listenfd)
            {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
// LT水平触发
#ifdef LT
                int cfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (cfd < 0)
                {

                    // LOG_ERROR("%s:errno is:%d", "accept error", errno);

                    continue;
                }
                // 如果文件描述符数量超过最大限制
                if (http_conn::m_user_count >= MAX_FD)
                {
                    show_error(cfd, "Internal server busy");

                    // LOG_ERROR("%s", "Internal server busy");

                    continue;
                }
                users[cfd].init(cfd, client_addr);

                // 初始化该连接对应的连接资源
                users_timer[cfd].address = client_addr;
                users_timer[cfd].sockfd = cfd;

                // 创建定时器临时变量
                util_timer *timer = new util_timer;
                // 设置定时器对应的连接资源
                timer->user_data = &users_timer[cfd];
                // 设置回调函数
                timer->cb_func = cb_func;
                time_t cur = time(NULL);
                // 设置绝对超过时间
                timer->expire = cur + 3 * TIMESLOT;

                // 创建该连接对应的定时器，初始化为前述临时变量
                users_timer[cfd].timer = timer;
                // 将该定时器添加到链表中
                timer_lst.add_timer(timer);

#endif
// ET非阻塞边缘触发
#ifdef ET
                // 需要循环接收数据
                while (1)
                {
                    int cfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
                    if (cfd < 0)
                    {

                        // LOG_ERROR("%s:errno is:%d", "accept error", errno);

                        break;
                    }
                    if (http_conn::m_user_count >= MAX_FD)
                    {
                        show_error(cfd, "Internal server busy");

                        // LOG_ERROR("%s", "Internal server busy");

                        break;
                    }
                    users[cfd].init(cfd, client_addr);

                    // 初始化该连接对应的连接资源
                    users_timer[cfd].address = client_addr;
                    users_timer[cfd].sockfd = cfd;

                    // 创建定时器临时变量
                    util_timer *timer = new util_timer;
                    // 设置定时器对应的连接资源
                    timer->user_data = &users_timer[cfd];
                    // 设置回调函数
                    timer->cb_func = cb_func;
                    time_t cur = time(NULL);
                    // 设置绝对超过时间
                    timer->expire = cur + 3 * TIMESLOT;

                    // 创建该连接对应的定时器，初始化为前述临时变量
                    user_timer[cfd].timer = timer;
                    // 将该定时器添加到链表中
                    timer_lst.add_timer(timer);
                }
                continue;
#endif
            }
            // 处理异常事件
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 服务器关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                timer->cb_func(&users_timer[sockfd]);
                if (timer)
                {
                    timer_lst.del_timer(timer);
                }
            }
            // 处理信号，events[i].events & EPOLLIN 用于判断是不是读事件，管道读端对应文件描述符发生读事件
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];

                // 从管道读端读出信号值，成功返回字节数，失败返回-1
                // 正常情况下，这里的 ret 返回值总是 1 ，只有 14 和 15 两个 ASCII 码对应的字符
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if (ret == -1)
                {
                    // handler the error
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    // 处理信号对应的逻辑
                    for (int i = 0; i < ret; i++)
                    {
                        // 这里面是字符
                        switch (signals[i])
                        {
                        // 这里是整型
                        case SIGALRM:
                        {
                            timeout = true;
                            break;
                        }
                        case SIGTERM:
                        {
                            stop_server = true;
                        }
                        }
                    }
                }
            }
            // 处理客户端连接上接收到的数据，这里判断是读事件，又不是listenfd上的读事件，那么就只能是客户端来了数据
            else if (events[i].events & EPOLLIN)
            {
                // 创建定时器临时变量，将该连接对应的定时器取出来
                util_timer *timer = users_timer[sockfd].timer;

                // 读入对应缓冲区
                if (users[sockfd].read_once())
                {

                    // LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));
                    // Log::get_instance()->flush();

                    // 若监测到读事件，将该读事件放入请求队列
                    pool->append(users + sockfd);

                    // 若有数据传输，则将定时器重新设置超时时间为 3 个单位
                    // 对其在链表上的位置进行调整
                    if (timer)
                    {
                        time_t cur = time(NULL);
                        timer->expire = cur + 3 * TIMESLOT;

                        // LOG_INFO("%s", "adjust timer once");
                        // Log::get_instance()->flush();

                        timer_lst.adjust_timer(timer);
                    }
                }
                else
                {
                    // 服务器关闭连接，移除对应的定时器
                    timer->cb_func(&users_timer[sockfd]);
                    if (timer)
                    {
                        timer_lst.del_timer(timer);
                    }
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                // 创建定时器临时变量，将该连接对应的定时器取出来
                util_timer *timer = users_timer[sockfd].timer;
                if (users[sockfd].write())
                {

                    // LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));
                    // Log::get_instance()->flush();

                    // 若有数据传输，则将定时器重新设置超时时间为 3 个单位
                    // 对其在链表上的位置进行调整
                    if (timer)
                    {
                        time_t cur = time(NULL);
                        timer->expire = cur + 3 * TIMESLOT;

                        // LOG_INFO("%s", "adjust timer once");
                        // Log::get_instance()->flush();

                        timer_lst.adjust_timer(timer);
                    }
                }
                else
                {
                    // 服务器关闭连接，移除对应的定时器
                    timer->cb_func(&users_timer[sockfd]);
                    if (timer)
                    {
                        timer_lst.del_timer(timer);
                    }
                }
            }
        }
        // 处理定时器为非必须事件，收到信号并不是立马处理
        // 完成所有读写事件后，再进行处理
        if (timeout)
        {
            timer_handler();
            timeout = false;
        }
    }
    close(epollfd);
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete pool;
    return 0;
}
