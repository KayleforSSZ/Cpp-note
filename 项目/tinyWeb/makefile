server: main.cpp ./threadpool/threadpool.h ./timer/lst_timer.h ./log/log.h ./log/log.cpp ./log/block_queue.h ./lock/mylock.h ./http/http_conn.h ./http/http_conn.cpp ./CGImysql/sql_connection_pool.h ./CGImysql/sql_connection_pool.cpp
	g++ -o server main.cpp ./threadpool/threadpool.h ./timer/lst_timer.h ./log/log.h ./log/log.cpp ./log/block_queue.h ./lock/mylock.h ./http/http_conn.h ./http/http_conn.cpp ./CGImysql/sql_connection_pool.h ./CGImysql/sql_connection_pool.cpp -lpthread -lmysqlclient -g

clean:
	rm -f server