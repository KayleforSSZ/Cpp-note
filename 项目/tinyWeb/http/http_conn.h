#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include "../lock/mylock.h"
#include "../CGImysql/sql_connection_pool.h"
class http_conn
{
public:
    // 设置文件名大小
    static const int FILENAME_BUFSIZ = 200;
    // 设置读缓冲区大小
    static const int READ_BUFSIZ = 2048;
    // 设置写缓冲区大小
    static const int WRITE_BUFSIZ = 1024;
    // 报文的请求方式，本项目中只使用了POST
    enum REQUEST_METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        CONNECT,
        POTIONS,
        TRACE
    };
    // 主状态机的状态
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0, // 解析请求行
        CHECK_STATE_HEADER,          // 解析请求头
        CHECK_STATE_CONTENT          // 解析消息体，仅用于解析 POST 请求，GET 请求没有消息体，GET 请求所有的参数都包含在请求行里
    };
    // 报文解析结果
    enum HTTP_CODE
    {
        NO_REQUEST,        // 请求不完整，需要继续读取请求报文数据 或者 跳转主线程继续监测事件
        GET_REQUEST,       // 获得了完整的 HTTP 请求，调用do_request完成请求资源映射
        BAD_REQUEST,       // HTTP 请求报文有语法错误或请求资源为目录，跳转process_write完成报文响应
        NO_RESOURCE,       // 请求资源不存在，跳转process_write完成报文响应
        FORBIDDEN_REQUEST, // 请求资源禁止访问，没有读取权限，跳转process_write完成报文响应
        FILE_REQUEST,      // 请求资源可以正常访问，跳转process_write完成报文响应
        INTERNAL_ERROR,    // 服务器内部错误，该结果在主状态机逻辑 switch 的 default 下，一半不会触发
        CLOSED_CONNECTION
    };
    // 从状态机状态
    enum LINE_STATUS
    {
        LINE_OK = 0, // 完整读取一行
        LINE_BAD,    // 报文语法错误
        LINE_OPEN    // 读取的行不完整
    };

public:
    http_conn() {}
    ~http_conn() {}

private:
    int m_sockfd;          // 服务器端socket
    sockaddr_in m_address; // 服务器端 socket 地址

    // 存储读取的请求报文数据
    char m_read_buf[READ_BUFSIZ];

    // 缓冲区中m_read_buf中数据的最后一个字节的下一个位置
    int m_read_idx;

    // m_read_buf 读取的位置 m_checked_idx
    int m_checked_idx;

    // m_read_buf 已经解析的字符个数
    int m_start_line;

    // 存储发出的响应报文的数据
    char m_write_buf[WRITE_BUFSIZ];

    // 指示 m_write_buf 中的长度
    int m_write_idx;

    // 主状态机的状态
    CHECK_STATE m_check_state;
    // 请求方法 GET or POST
    REQUEST_METHOD m_request_method;

    // 以下为解析请求报文中对应的6个变量
    // 存储读取文件的名称
    char m_real_file[FILENAME_BUFSIZ];
    char *m_url;          // url
    char *m_version;      // HTTP版本号
    char *m_host;         // host号
    int m_content_length; // 消息体长度
    bool m_linger;        // 是不是长连接

    char *m_file_address;    // 读取服务器上的文件地址
    struct stat m_file_stat; // inode结构体，用于获取文件属性，主要用于大小，类型
    struct iovec m_iv[2];    // io 向量机制iovec
    int m_iv_count;
    int cgi;             // 是否启用POST
    char *m_string;      // 存储请求头数据
    int bytes_to_send;   // 剩余发送字节数
    int bytes_have_send; // 已发送字节数

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;

private:
    // 私有成员变量初始化
    void init();
    // 从 m_read_buf 中读取，并解析请求报文，返回报文处理结果
    HTTP_CODE process_read();
    // 向 m_write_buf 中写入响应报文数据，返回写入成功与否结果
    bool process_write(HTTP_CODE ret);
    // 主状态机解析报文中的请求《行》数据
    HTTP_CODE parse_request_line(char *text);
    // 主状态机解析报文中的请求《头》数据
    HTTP_CODE parse_request_headers(char *text);
    // 主状态机解析报文中的请求内容
    HTTP_CODE parse_content(char *text);
    // 生成响应报文
    HTTP_CODE do_request();

    // m_start_line 是 m_read_buf 已经解析的字符个数
    // getline 用于将指针向后偏移，指向未处理的字符
    char *get_line() { return m_read_buf + m_start_line; }

    // 从状态机读取一行，分析是请求报文的哪一部分
    LINE_STATUS parse_line();

    void unmap();

    // 根据响应报文格式，生成对应8个部分，一下函数均由 do_request 调用
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    // 初始化套接字地址，函数内部会调用私有方法 init
    void init(int sockfd, const struct sockaddr_in &addr);
    // 关闭 http 连接
    void close_conn(bool real_close = true);
    // 各子线程处理任务函数
    void process();
    // 读取浏览器发来的全部数据
    bool read_once();
    // 响应报文写入函数
    bool write();
    sockaddr_in *get_address() { return &m_address; }
    // 同步线程初始化数据库读取表
    void initmysql_result(connection_pool *connPool);
};

#endif