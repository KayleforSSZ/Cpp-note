#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

connection_pool::connection_pool()
{
    this->CurConn = 0;
    this->FreeConn = 0;
}

// RAII机制销毁连接池
connection_pool::~connection_pool()
{
    DestroyPool();
}

// 单例模式创建连接池
connection_pool *connection_pool::GetInstance()
{
    static connection_pool connPool;
    return &connPool;
}

// 初始化
void connection_pool::init(string url, string user, string PassWd, string databasename, int port, unsigned int MaxConn)
{
    // 初始化数据库信息
    this->url = url;
    this->User = user;
    this->PassWard = PassWd;
    this->Port = port;
    this->DatabaseName = databasename;

    lock.lock();
    // 创建 MaxConn 条数据库连接
    for (int i = 0; i < MaxConn; i++)
    {
        MYSQL *con = NULL;
        con = mysql_init(con);

        if (con == NULL)
        {
            cout << "Error: " << mysql_error(con);
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), user.c_str(), PassWd.c_str(), databasename.c_str(), port, NULL, 0);
        if (con == NULL)
        {
            cout << "Error: " << mysql_error(con);
            exit(1);
        }

        // 更新连接池和空闲连接数量
        connList.push_back(con);
        ++FreeConn;
    }

    // 将信号量初始化为最大连接次数
    reserve = sem(FreeConn);

    this->MaxConn = FreeConn;

    lock.unlock();
}

// 当有请求时，从数据库连接池中返回一个可用链表，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection()
{
    MYSQL *con = NULL;
    if (connList.size() == 0)
        return NULL;

    // 取出连接，信号量原子减 1，为 0 则等待
    reserve.wait();

    lock.lock();

    con = connList.front();
    connList.pop_front();

    // 这里的两个变量并没有用到
    --FreeConn;
    ++CurConn;

    lock.unlock();
    return con;
}

// 释放当前使用的连接
bool connection_pool::ReleaseConnection(MYSQL *con)
{
    if (con == NULL)
        return false;

    lock.lock();
    connList.push_back(con);
    ++FreeConn;
    --CurConn;
    lock.unlock();

    reserve.post();

    return true;
}

// 当前空闲连接数量
int connection_pool::GetFreeConn()
{
    return this->FreeConn;
}

// 销毁所有连接
void connection_pool::DestroyPool()
{
    lock.lock();
    if (connList.size() > 0)
    {
        // 遍历关闭数据库连接
        list<MYSQL *>::iterator it;
        for (it = connList.begin(); it != connList.end(); it++)
        {
            MYSQL *con = *it;
            mysql_close(con);
        }
        CurConn = 0;
        FreeConn = 0;

        // 清空 list
        connList.clear();
    }
    lock.unlock();
}

// 不直接调用获取和释放连接的接口，将其封装起来，通过 RAII 机制进行获取和释放
connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool)
{
    *SQL = connPool->GetConnection();

    connRAII = *SQL;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII()
{
    poolRAII->ReleaseConnection(connRAII);
}