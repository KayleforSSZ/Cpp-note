#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/mylock.h"

using namespace std;

class connection_pool
{
private:
    string url;          // 主机地址
    string Port;         // 数据库端口号
    string User;         // 登录数据库用户名
    string PassWard;     // 登录数据库密码
    string DatabaseName; // 使用数据库名

private:
    mylock lock;
    list<MYSQL *> connList; // 连接池
    sem reserve;            // 初始化为数据库的连接总数

private:
    unsigned int MaxConn;  // 最大连接数
    unsigned int CurConn;  // 当前已使用的连接数
    unsigned int FreeConn; // 当前空闲连接数

public:
    MYSQL *GetConnection();              // 获取数据库连接
    bool ReleaseConnection(MYSQL *conn); // 释放连接
    int GetFreeConn();                   // 获取连接
    void DestroyPool();                  // 销毁所有连接，通过RAII机制来完成自动释放

    // 单例模式
    static connection_pool *GetInstance();
    // 初始化
    void init(string url, string user, string PassWd, string databasename, int port, unsigned int MaxConn);

    connection_pool();
    ~connection_pool();
};

class connectionRAII
{
private:
    MYSQL *connRAII;
    connection_pool *poolRAII;

public:
    // 双指针对 MYSQL *con 修改
    connectionRAII(MYSQL **con, connection_pool *connPool);
    ~connectionRAII();
};

#endif
