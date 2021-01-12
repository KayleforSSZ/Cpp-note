## ==一、MYSQL逻辑架构
### 1. 连接层

**最上层是一些客户端和连接服务，包含本地sock通信和大多数基于客户端/服务端工具实现的类似于TCP/IP的通信**。主要完成一些类似于连接处理、授权认证及相关的安全方案。在该层上引入了线程池的概念，为通过认证安全接入的客户提供线程。同样在该层上可以实现基于SSL的安全连接。服务器也会为安全接入的每个客户端验证它所具有的操作权限。

### 2. 服务层
**第二层架构主要完成大多数的核心功能，如SQL接口，并完成缓存的查询，SQL的分析和优化及部分内置函数的执行。**所有跨存储引擎的功能也在这一层实现，如过程、函数等。在该层，服务器会解析查询并创建相应的内部解析树，并对其完成相应的优化如确定查询表的顺序，是否利用索引等。最后生成相应的执行操作。如果是select语句，服务器还会查询内部的缓存。如果缓存空间足够大，这样在解决大量读操作的环境中能够很好的提升系统的性能。

查询缓存 —> 语法解析 —> 查询优化

### 3. 引擎层
**存储引擎层，真正的负责了MySQL中数据的存储和提取，服务器通过API与存储引擎进行通信。**不同的存储引擎具有的功能不同，这样我们可以根据自己的实际需要进行选取。

### 4. 存储层
**数据存储层，主要是讲数据存储在运行于裸设备的文件系统之上，并完成与存储层引擎的交互。**

## 二、MySQL存储引擎
### 1. MyISAM和InnoDB

| 对比项   | MyISAM                                                       | InnoDB                                                       |
| -------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 主外键   | 不支持                                                       | 支持                                                         |
| 事务     | 不支持                                                       | 支持                                                         |
| 行表锁   | 表锁，即使操作一条记录也会锁住<br />整个表，不适合高并发的操作 | 行锁，操作时只锁某一行，不对其它行有影<br />响，**适合高并发的操作** |
| 缓存     | 只缓存索引，不缓存真实数据                                   | 不仅缓存索引还要缓存真实数据，对内存要<br />求较高，并且内存大小对性能有决定性的影响 |
| 表空间   | 小                                                           | 大                                                           |
| 关注点   | 性能                                                         | 事务                                                         |
| 默认安装 | Y                                                            | Y                                                            |

## 三、MySQL索引优化

### 1. 性能下降SQL慢原因

- **查询语句写的烂**

- **索引失效**

索引分为**单值**索引或**复合**索引，建索引之后内部会做排序，查询更快

```mysql
# 假设user表中有id、name、email、number字段
select * from name where name = 'zhangsan';
# 根据name字段设置单值索引，查询会更快
create index idx_user_name on user(name);

select * from name where name = 'zhangsan' and email = '123@qq.com';
# 根据name和emain字段设置联合索引
create index idx_user_nameEmail on user(name, email);
```

- **关联查询太多join（设计缺陷或不得已的需求）**

- **服务器调优及各个参数设置（缓冲、线程数等）**

### 2. join查询

##### SQL执行顺序

- **手写**

```mysql
# 查询一般手写代码逻辑
SELECT DISTINCT
	< select_list >
FROM
	< left_table > < join_type >
JOIN < right_table > ON < join_condition >
WHERE
	< where_condition >
GROUP BY
	< group_by_list >
HAVING
	< having_condition >
ORDER BY
	< order_by_condition >
LIMIT < limit_number >
```

- **机读**

```mysql
# 机读顺序
FROM < left_table >
ON < join_condition >
< join_type > JOIN < right_table >
WHERE < where_condition >
GROUP BY < group_by_list >
HAVING < having_condition >
SELECT
DISTINCT < select_list >
ORDER BY < order_by_condition >
LIMIT < limit_number >
```

- **总结**

![](/图/SQL解析.png)

##### join图

<img src="图/join查询.jpg" style="zoom:150%;" />

### 3. 索引简介

##### 索引的定义

- 索引（Index）是帮助MySQL高效获取数据的==**数据结构**==。索引的本质是数据结构。索引的==目的==在于<u>提高查找效率</u>，可以类比字典。可以简单理解为==排好序的快速查找**数据结构**==。

- 在数据之外，数据库系统还**维护着满足特定查找算法的数据结构**，这些数据结构以某种方式引用(指向)数据，这样就可以在这些数据结构上实现高级查找算法。这种数据结构，就是**索引**。

- 一般来说，索引本身也很大，不可能全部存储在内存中，因此索引往往以索引文件的形式存储在磁盘上。

- 我们平常所说的索引，如果没有特别指明，都是指==B树==（多路搜索树）结构组织的索引。其中**聚集索引、次要索引、覆盖索引、复合索引、前缀索引、唯一索引**默认都是使用==B+树==，统称索引。当然，除了B+树这种索引外，还有**hash索引**等。

##### 索引的优势

- 类似大学图书馆建书目索引，提高数据检索的效率，降低数据库的IO成本
- 通过索引列对数据进行排序，降低数据排序的成本，降低了CPU的消耗

##### 索引的劣势

- 索引也是一张表，该表保存了主键与索引字段，并指向实体表的记录，所以索引列也是要占用空间的。

- 虽然索引大大提高了查询速度，同时却降低了更新表的速度，如对表进行**INSERT**、**UPDATE**和**DELETE**。因为更新表时，MySQL不仅要保存数据，还要保存一下索引文件每次更新添加了索引列的字段，都会调整因为更新所带来的键值变化后的索引信息。
- 索引只是提高效率的一个因素，如果你的MySQL有大数据量的表，就需要花时间研究建立最优秀的索引，或优化查询。

##### MySQL索引分类

- **单值索引**：一个索引只包含单个列，一个表可以有多个单列索引

- **唯一索引**：索引列的值必须唯一，但允许有空值

- **复合索引**：一个索引包含多个列

- **基本语法**

	- 创建

		- CREATE [UNIQUE] INDEX indexName ON mytable(columnname(length));
		- ALTER mytable ADD [UNIQUE] INDEX [indexName] ON (olumnname(length));

	- 删除

		- DROP INDEX [indexName] ON mytable;

	- 查看

		- SHOW INDEX FROM table_name\G

	- 使用ALTER命令

		- 有四种方式来添加数据表的索引

		```mysql
		# 该语句添加一个主键，这意味着索引值必须唯一，且不能为NULL
		ALTER TABLE tbl_name ADD PRIMARY KEY(column_list);
		# 这条语句创建索引的值必须是唯一的（除了NULL外，NULL可能出现多次）
		ALTER TABLE tbl_name ADD UNIQUE index_name(column_list);
		# 添加普通索引，索引值可出现多次
		ALTER TABLE tbl_name ADD INDEX index_name(column_list);
		# 该语句制定了索引为FULLTEXT，用于全文索引
		ALTER TABLE tbl_name ADD FULLTEXT index_name(column_list);
		```

##### MySQL索引结构

- BTree索引
	- 数据只存在于叶子节点，会大大减少IO次数，非叶节点存储“药引”和指针
- Hash索引
- full-text全文索引
- R-Tree索引

##### 哪些情况需要创建索引

1. 主键自动建立唯一索引
2. 频繁作为查询条件的字段应该创建索引（比如银行系统里的账号，电信系统里的手机号）
3. 查询中与其他表关联的字段，外键关系建立索引
4. 频繁更新的字段不适合创建索引，因为每次更新不单单更新数据还要更新索引
5. where条件里用不到的字段不要创建索引
6. 单键/组合索引的选择问题（在高并发下倾向于创建组合索引）
7. 查询中排序的字段，排序字段若通过索引去访问将大大提高排序速度
8. 查询中统计或者分组字段

##### 哪些情况需要创建索引

1. 表记录太少
2. 经常增删改的表（提高了查询表的速度，同时却降低更新表的速度）
3. 数据重复且分布平均的表字段，因此应该只为最经常查询和最经常排序的数据列建立索引。注意，如果某个数据列包含许多重复的内容，为它建立索引就没有太大的实际效果。

### 4. 性能分析

##### MySQL Query Optimizer 查询优化器

- MySQL中有专门负责优化SELECT语句的优化器模块，主要功能：通过计算分析系统中收集到的统计信息，为客户端请求的Query提供它认为的最优的执行计划（它认为最优的数据检索方式，但不见得是DBA认为的最优的，这部分最耗时间）
- 当客户端向MySQL请求一条Query，命令解析器模块完成请求分类，区别出是SELECT并转发给MySQL Query Optimizer时，MySQL Query Optimizer首先会对整条Query进行优化，处理掉一些常量表达式的预算，直接换算成常量值。并对Query中的查询条件进行简化和转换，如去掉一些无用或显而易见的条件、结构调整等。然后分析Query中的Hint信息（如果有），看显示Hint信息是否可以完全确定该Query的执行计划。如果没有Hint或Hint信息还不足以完全确定执行计划，则会读取所涉及对象的统计信息，根据Query进行写相应的计算分析，然后再得出最后的执行计划。

##### MySQL常见瓶颈

- CPU：CPU在饱和的时候一般繁盛在数据装入内存或从磁盘上读取数据的时候
- IO：磁盘IO瓶颈发生在装入数据远远大于内存容量的时候
- 服务器硬件的性能瓶颈：top，free，iostat 和 vmstat 来查看系统的性能状态

##### Explain

- 是什么（查看执行计划）

	使用EXPLAIN关键字可以模拟优化器执行SQL查询语句，从而指导MySQL是如何处理你的SQL语句的。分析你的查询语句或是表结构的性能瓶颈。

- 能干嘛
	- 表的读取顺序（ id ）
	- 数据读取操作的操作类型 （ select_type ）
	- 哪些索引可以使用（ possible_key ）
	- 哪些索引被实际使用（ key ）
	- 表之间的引用（ ref ）
	- 每张表有多少行被优化器查询（ rows ）

- 怎么玩
	- EXPLAIN + SQL语句

	- 执行计划包含的信息

		![](图/explain字段.png)

- 各字段解释

	- ==**id**==

		- select 查询的序列号，包含一组数字，表示查询中执行 select 子句或操作表的顺序
		- 三种情况
			- id 相同：执行顺序由上至下
			- id 不同：如果是子查询，id 的序号会递增，**id 值越大优先级越高，越先被执行**
			- id 有相同的也有不同的：id 相同的自上而下执行，id值越大越先被执行

	- ==**select_type**==

		- 有哪些
			- **SIMPLE**：简单查询，查询中不包含子查询或UNION
			- **PRIMARY**：查询中若包含复杂的子部分，最外层查询则被标记为PRIMARY（最后加载的）
			- **SUBQUERY**：在SELECT或WHERE列表中包含了子查询
			- **DERIVED**：在FROM列表中包含的子查询被标记为DERIVED(衍生)，MySQL会递归进行执行这些子查询，把结果放在==临时表==里。
			- **UNION**：若第二个SELECT出现在UNION之后，则被标记为UNION，若UNION包含在FROM子句的子查询中，外层SELECT将被标记为：DERIVED
			- **UNION RESULT**：从UNION表获取结果的SELECT
		- 查询的类型，主要用于区别普通查询、联合查询、子查询等的复杂查询

	- ==**table**==

		- 显示这一行的数据是关于哪张表的

	- ==**type**==

		- 访问类型排列

		- 从最好到最差依次是：

			system > const > eq_ref > ref > range > index > ALL

			==一般来说，得保证查询至少达到range级别，最好能达到ref==

		| 类型   | 描述                                                         |
		| ------ | ------------------------------------------------------------ |
		| system | 表中只有一行（等于系统表），这时const类型的特例，平时不会出现，可以忽略 |
		| const  | 表示通过索引一次就找到了，const用于比较primary key或者unique索引，因为只匹配一行数据，所以很快，如将主键置于where列表中，MYSQL就能将该查询转换为一个常量 |
		| eq_ref | 唯一索引扫描，对于每个索引键，表中只有`一条记录`与之匹配。常见于主键或唯一索引扫描。 |
		| ref    | 非唯一性索引扫描，返回匹配某个单独值的`所有行`。本质上也是一种索引访问，它返回所有匹配某个单独值的行，然而，它可能会找到多个符合条件的行，所以它应该属于查找和嫂买哦得混合体。 |
		| range  | 只检索给定范围的行，使用一个索引来选择行。key列显示使用了哪个索引，一般就是在你的where语句中出现了between、<、>、in等的查询，这种范围扫描索引扫描比全表扫描要好，因为它只需要开始于索引的某一点，而结束于另一点，不用扫描全部索引。 |
		| index  | Full Index Scan，index与ALL区别为index类型只遍历索引树。这通常比ALL快，因为索引文件永昌比数据文件小（也就是说虽然ALL和index都是读全表，但是index是从索引中读取的，而all是从硬盘中读取的） |
		| ALL    | Full Table Scan，将遍历全表以找到匹配的方案                  |

		- ==possible_keys==
			- 显示可能应用在这张表中的索引，一个或多个查询涉及到的字段上若存在多个索引，则该索引将被列出，`但不一定被查询实际用到`
		- ==key==
			- `实际使用的索引`。如果为NULL，则没有使用索引。
			- 查询中若使用了覆盖索引，则该索引仅出现在key列表中
		- ==key_len==
			- 表示索引中使用的字节数，可通过该列计算查询中使用的索引的长度。在不损失精确性的情况下，长度越短越好。
			- key_len显示的值为索引字段的最大可能长度，`并非实际使用长度`，即key_len是根据表定义计算而得，不是通过表内检索出的
		- ==ref==
			- 显示索引的哪一列被使用了，如果可能的话，是一个常数。哪些列或常量被用于查找索引列上的值
		- ==rows==
			- 根据表统计信息及索引选用情况，大致估算出找到所需的记录所需要读取的行数
		- ==Extra==
			- 包含不适合在其他列中显示但十分重要的额外信息

		| 类型                         | 描述                                                         |
		| ---------------------------- | ------------------------------------------------------------ |
		| ==Using filesort==           | 说明mysql会对数据使用一个外部的索引排序，而不是按照表内的索引顺序<br />进行读取的，mysql中无法利用索引完成的排序操作称为“文件排序”。常见<br />于order by |
		| ==Using temporary==          | 使用了临时表保存中间结果，mysql在对查询结果排序时使用临时表。常见<br />于order by和分组查询group by。（很伤害系统性能） |
		| ==Using index==              | 表明相应的select操作中使用了覆盖索引，避免访问了表的数据行，效率不<br />错！如果同时出现Using where，表明索引被用来执行索引键值的查找；如<br />果没有同时出现Using where，表明索引用来读取数据而非执行查找操作。<br />`覆盖索引`：就是select的数据只用从索引中就能够取得，不必读取数据<br />行，MySQL可以利用索引返回select列表中的字段，而不必根据索引再次<br />读取数据文件，换句话说查询列表要被所建的索引覆盖。 |
		| Using where                  | 表明使用了where过滤                                          |
		| Using join buffer            | 使用了连接缓存                                               |
		| impossible where             | where子句的值总是false，不能用来获取任何元组                 |
		| select tavles optimized away | 在没有GROUP BY子句的情况下，基于索引优化MIN/MAX操作或者对于<br />MyISAM存储引擎优化COUNT(*)操作，不必等到执行阶段再进行计算，<br />查询执行计划生成的阶段即完成优化。 |
		| distinct                     | 优化distinct操作，在找到第一匹配的元组后即停止找同样值的动作 |

### 5. 索引优化

##### 单表案例

```mysql
# 建表
CREATE TABLE IF NOT EXISTS `article`(
`id` INT(10) UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
`author_id` INT (10) UNSIGNED NOT NULL,
`category_id` INT(10) UNSIGNED NOT NULL , 
`views` INT(10) UNSIGNED NOT NULL , 
`comments` INT(10) UNSIGNED NOT NULL,
`title` VARBINARY(255) NOT NULL,
`content` TEXT NOT NULL
);

# 查询 category_id 为1且comments大于1的情况下，views最多的id
explain select id from article where category_id = 1 and comments > 1 order by views desc limit 1;

+----+-------------+---------+------------+------+---------------+------+---------+------+------+----------+-----------------------------+
| id | select_type | table   | partitions | type | possible_keys | key  | key_len | ref  | rows | filtered | Extra                       |
+----+-------------+---------+------------+------+---------------+------+---------+------+------+----------+-----------------------------+
|  1 | SIMPLE      | article | NULL       | ALL  | NULL          | NULL | NULL    | NULL |    3 |    33.33 | Using where; Using filesort |
+----+-------------+---------+------------+------+---------------+------+---------+------+------+----------+-----------------------------+
1 row in set, 1 warning (0.00 sec)
```

- ==结论==：很显然，type为ALL，即最坏的情况。Extra里还出现了Using filesort，也是最坏的情况。优化是必须的。

- `第一次优化`：由于用到了category_id、comments和views这三个字段，所以先用这三个字段建索引

```mysql
create index idx_article_ccv on article(category_id,comments,views);
explain select id from article where category_id = 1 and comments > 1 order by views desc limit 1;
+----+-------------+---------+------------+-------+-----------------+-----------------+---------+------+------+----------+------------------------------------------+
| id | select_type | table   | partitions | type  | possible_keys   | key             | key_len | ref  | rows | filtered | Extra                                    |
+----+-------------+---------+------------+-------+-----------------+-----------------+---------+------+------+----------+------------------------------------------+
|  1 | SIMPLE      | article | NULL       | range | idx_article_ccv | idx_article_ccv | 8       | NULL |    1 |   100.00 | Using where; Using index; Using filesort |
+----+-------------+---------+------------+-------+-----------------+-----------------+---------+------+------+----------+------------------------------------------+
1 row in set, 1 warning (0.00 sec)
```

- 此时可以看出，用到了索引，解决了 type为ALL 的全表扫描的问题，但依然没解决Using filesort的问题。因为 commets>1 的条件使后面的索引失效，如果改成comments = 1，那么Using filesort就会消失。


- 我们删掉索引重新来

```mysql
drop index idx_article_ccv on article;
```

- 第二次优化，去掉comments的索引，使用category_id和views建立索引


```mysql
create index idx_article_cv on article(category_id,views);
explain select id from article where category_id = 1 and comments > 1 order by views desc limit 1;
+----+-------------+---------+------------+------+----------------+----------------+---------+-------+------+----------+-------------+
| id | select_type | table   | partitions | type | possible_keys  | key            | key_len | ref   | rows | filtered | Extra       |
+----+-------------+---------+------------+------+----------------+----------------+---------+-------+------+----------+-------------+
|  1 | SIMPLE      | article | NULL       | ref  | idx_article_cv | idx_article_cv | 4       | const |    2 |    33.33 | Using where |
+----+-------------+---------+------------+------+----------------+----------------+---------+-------+------+----------+-------------+
1 row in set, 1 warning (0.00 sec)
```

- 再看，type变为ref，Using filesort也消失了，优化成功。


##### 两表案例

```mysql
# 建表
CREATE TABLE IF NOT EXISTS `class`(
`id` INT(10) UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
`card` INT (10) UNSIGNED NOT NULL
);
CREATE TABLE IF NOT EXISTS `book`(
`bookid` INT(10) UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
`card` INT (10) UNSIGNED NOT NULL
);

# 左连接查询
explain select * from class left join book on class.card = book.card;
+----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+----------------------------------------------------+
| id | select_type | table | partitions | type | possible_keys | key  | key_len | ref  | rows | filtered | Extra                                              |
+----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+----------------------------------------------------+
|  1 | SIMPLE      | class | NULL       | ALL  | NULL          | NULL | NULL    | NULL |   20 |   100.00 | NULL                                               |
|  1 | SIMPLE      | book  | NULL       | ALL  | NULL          | NULL | NULL    | NULL |   20 |   100.00 | Using where; Using join buffer (Block Nested Loop) |
+----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+----------------------------------------------------+
2 rows in set, 1 warning (0.00 sec)
```

- 可以看到，两个type都是ALL
- `第一次优化`，使用book表的card建索引

```mysql
alter table book add index y(card);
explain select * from class left join book on class.card = book.card;
+----+-------------+-------+------------+------+---------------+------+---------+-------------------+------+----------+-------------+
| id | select_type | table | partitions | type | possible_keys | key  | key_len | ref               | rows | filtered | Extra       |
+----+-------------+-------+------------+------+---------------+------+---------+-------------------+------+----------+-------------+
|  1 | SIMPLE      | class | NULL       | ALL  | NULL          | NULL | NULL    | NULL              |   20 |   100.00 | NULL        |
|  1 | SIMPLE      | book  | NULL       | ref  | y             | y    | 4       | db0720.class.card |    1 |   100.00 | Using index |
+----+-------------+-------+------------+------+---------------+------+---------+-------------------+------+----------+-------------+
2 rows in set, 1 warning (0.00 sec)
```

- book表的type已经变成ref了，用上了索引


- `第二次尝试`，使用class表的card建索引

```mysql
 drop index y on book;
 alter table class add index y (card);
 explain select * from class left join book on class.card = book.card;
 
+----+-------------+-------+------------+-------+---------------+------+---------+------+------+----------+----------------------------------------------------+
| id | select_type | table | partitions | type  | possible_keys | key  | key_len | ref  | rows | filtered | Extra                                              |
+----+-------------+-------+------------+-------+---------------+------+---------+------+------+----------+----------------------------------------------------+
|  1 | SIMPLE      | class | NULL       | index | NULL          | y    | 4       | NULL |   20 |   100.00 | Using index                                        |
|  1 | SIMPLE      | book  | NULL       | ALL   | NULL          | NULL | NULL    | NULL |   20 |   100.00 | Using where; Using join buffer (Block Nested Loop) |
+----+-------------+-------+------------+-------+---------------+------+---------+------+------+----------+----------------------------------------------------+
2 rows in set, 1 warning (0.00 sec)
```

- 注意看，这一次class表的type变成了index，而index是不如ref的，通过观察rows也知道，是20+20，不如上面的


- ==结论==：**左连接建右表，右连接建左表**。

##### 三表案例

```mysql
# 新增一个表
CREATE TABLE IF NOT EXISTS `phone`(
`phoneid` INT(10) UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
`card` INT (10) UNSIGNED NOT NULL
)ENGINE = INNODB;

# 三表左连接查询
explain select * from class left join book on class.card=book.card left join phone on book.card=phone.card;
+----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+----------------------------------------------------+
| id | select_type | table | partitions | type | possible_keys | key  | key_len | ref  | rows | filtered | Extra                                              |
+----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+----------------------------------------------------+
|  1 | SIMPLE      | class | NULL       | ALL  | NULL          | NULL | NULL    | NULL |   20 |   100.00 | NULL                                               |
|  1 | SIMPLE      | book  | NULL       | ALL  | NULL          | NULL | NULL    | NULL |   20 |   100.00 | Using where; Using join buffer (Block Nested Loop) |
|  1 | SIMPLE      | phone | NULL       | ALL  | NULL          | NULL | NULL    | NULL |   20 |   100.00 | Using where; Using join buffer (Block Nested Loop) |
+----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+----------------------------------------------------+
3 rows in set, 1 warning (0.00 sec)

```

- 可知，三个表的type全都是ALL
- ==优化==，对每个表的card字段都建一个索引

```mysql
alter table book add index x(card);
alter table class add index y(card);
alter table phone add index z(card);
explain select * from class left join book on class.card=book.card left join phone on book.card=phone.card;

+----+-------------+-------+------------+-------+---------------+------+---------+-------------------+------+----------+-------------+
| id | select_type | table | partitions | type  | possible_keys | key  | key_len | ref               | rows | filtered | Extra       |
+----+-------------+-------+------------+-------+---------------+------+---------+-------------------+------+----------+-------------+
|  1 | SIMPLE      | class | NULL       | index | NULL          | y    | 4       | NULL              |   20 |   100.00 | Using index |
|  1 | SIMPLE      | book  | NULL       | ref   | x             | x    | 4       | db0720.class.card |    1 |   100.00 | Using index |
|  1 | SIMPLE      | phone | NULL       | ref   | z             | z    | 4       | db0720.book.card  |    1 |   100.00 | Using index |
+----+-------------+-------+------------+-------+---------------+------+---------+-------------------+------+----------+-------------+
3 rows in set, 1 warning (0.00 sec)
```

- 可以看到 book 和 class 的 type 字段已经变成 ref 了，rows字段也变成了1。效果不错，索引最好设置在需要经常查询的字段。


##### Join语句的优化

- 尽可能减少Join语句中嵌套循环的循环总次数；“永远用小结果集驱动大结果集”。

- 优先优化嵌套循环的内层循环。

- 保证Join语句中被驱动表上Join条件字段已经被索引。

- 当无法保证驱动表的Join条件字段被索引且内存资源充足的前提下，不要太吝啬JoinBuffer的设置。

##### 索引失效（应该避免）

1. 全值匹配我最爱

2. 最佳左前缀法则

	查询从索引的最左前列开始并且不跳过索引中的列（带头大哥不能死，中间兄弟不能断）

3. 不在索引列上做任何操作（计算、函数、类型转换），会导致索引失效而转向全表扫描（索引列上无计算）

4. 存储引擎不能使用索引中范围条件右边的列（范围之后全失效）

5. 尽量使用覆盖索引（只访问索引的查询（索引列和查询列一致）），减少select *（覆盖索引不写星）

6. MySQL在使用不等于（!= 或<>）的时候无法使用索引会导致全表扫描（）

7. is null ，is not null 也无法使用索引

8. like 以通配符开头（‘%abc...’）MySQL索引失效会变成全表扫描的操作（like%加右边）

	“...%”索引不会失效（部分有用），如果一定要前后都有%，那么可以使用覆盖索引来解决索引失效

9. 字符串不加单引号索引失效（字符串内有引号）

10. 少用or，用它来连接时会索引失效

==口诀==

全值匹配我最爱，最左前缀要遵守；

带头大哥不能死，中间兄弟不能断；

索引列上少计算，范围之后全失效；

百分Like加右边，覆盖索引不写星；

不等空值还有or，索引失效要少用；

VAR引号不可丢，SQL高级也不难！

## 四、查询截取分析

### 大纲

1. 慢查询的开启并捕获
2. explain+慢SQL分析
3. show profile查询SQL在MySQL服务器里的执行细节和生命周期情况
4. SQL数据库服务器的参数调优

### 1. 查询优化

##### 永远小表驱动大表

==优化原则==：小表驱动大表，即小的数据集驱动大的数据集

```mysql
select * from A where id in (select id from B);
等价于
for(select id from B)
for(select * from A where A.id = B.id)
```

当B表的数据集小于A表的数据集时，用in优于exists

```mysql
select * from A where exists (select 1 from B where B.id = A.id);
等价于
for(select * from A)
for(select * from B where B.id = A.id)
即
```

当A表的数据集小于B表的数据集时，用exists优于in

==exists关键字==

```mysql
select ... from table where exists (subquery);
```

该语法可以理解为：`将主查询的数据，放到子查询中做条件验证，根据验证结果（true或false）来决定主查询的数据结果是否得以保留。`

##### order by关键字优化

- **尽量使用Index方式排序，避免使用FileSort方式排序**
	- MySQL支持两种排序，FileSort和Index，Index效率高，它指MySQL扫描索引本身完成排序。FileSort效率低
	- ORDER BY满足两情况会使用Index排序
		- ORDER BY语句使用索引最左前列
		- 使用where子句与Order By字句条件组合满足索引最左前列

- **尽可能在索引列上完成排序操作，遵照索引建的最佳左前缀**

- **如果不在索引列上，filesort有两种算法，双路排序、单路排序**
	- ==双路排序==
		- MySQL 4.1之前是使用双路排序，字面意思就是两次扫描磁盘，最终得到数据，读取行指针和order by 列，对它们进行排序，然后扫描已经排序好的列表，按照列表中的值重新从列表中读取对应的数据输出
		- 从磁盘读取排序字段，在buffer进行排序，再从磁盘取其他字段
	- ==单路排序==
		- 从磁盘读取查询所需要的所有列，按照order by列在buffer对它们进行排序，然后扫描已经排序好的列表，它的效率更快一些，避免了第二次读取数据。并且把随机IO变成了顺序IO，但是它会使用更多的空间，因为它把每一行都保存在内存中了
		- 由于单路是后出的，总体而言是好过双路的
		- 但是单路有问题
			- 在sort_buffer中，方法B比方法A要占用更多的空间，因为方法B是把所有的字段都取出来，所以有可能去除的数据的总大小超过了sort_buffer的容量，导致每次只能取出sort_buffer容量大小的数据，进行排序（创建tmp文件，多路合并），排序完再取sort_buffer容量大小的数据，再排。。。从而导致多次IO，得不偿失。

- **优化策略**
	- 增大sort_buffer_size参数的设置
	- 增大max_length_of_sort_data参数的设置
- **提高order by的速度**
	- order by 时 select * 是一个大忌，只query需要的字段，这点非常重要。在这里的影响是：
		- 当 query 的字段大小总和小于 max_length_of_sort_data 而且排序字段不是 TEXT|BLOB 类型时，会用改进后的算法——单路排序，否则使用————多路算法。
		- 两种算法的数据都有可能超过 sort_buffer 的容量，超出之后，会创建tmp文件进行合并排序，导致多次IO，但是用单路排序算法的风险会更大一些，所以要提高 sort_buffer_size。
	- 尝试提高 sort_buffer_size
		- 不管用哪种算法，提高这个参数都会提高效率，当然，要根据系统的能力去提高，因为这个参数是针对每个进程
	- 尝试提高 max_length_of_sort_data
		- 提高这个参数，会增加用改进算法的概率。但是如果设置的太高，数据总容量超 sort_buffer_size 的概率就会增大，明显症状是高的磁盘IO活动和低的CPU使用率

- `为排序使用索引（重点）`

	- MySQL两种排序方式：文件排序、扫描有序索引排序
	- MySQL能为排序与查询使用相同的索引

	```mysql
	# 设置联合索引
	KEY a_ba_c(a, b, c);
	# order by 能使用索引，最左前缀
	order by a;
	order by a,b;
	order by a,b,c;
	order by a DESC, b DESC, c DESC;
	# 如果 where 使用索引的最左前缀定义为常量，则 order by 能使用索引
	where a = const order by b,c;
	where a = const and b = const order by c;
	where a = const and b > const order by b,c;
	# 不能使用索引进行排序
	order by a ASC, b DESC, c DESC;	/* 排序不一致 */
	where g = const order by b, c;	/* 丢失 a 索引 */
	where a = const order by c;		/* 丢失 b 索引 */
	where a = const order by a, d;	/* d 不是索引的一部分 */
	where a in(...) order by b, c;	/* 对于排序来说，多个想等条件也是范围查询 */
	```

##### group by关键字优化

- group by 实质是先排序再进行分组，遵照索引建的最佳左前缀
- 当无法使用索引项，增大max_length_for_sort_data参数的设置 + 增大sort_buffer_size参数的设置
- where 高于 having，能写在where限定的条件就不要去having限定了

### 2. 慢查询日志

##### 是什么

- MySQL的慢查询日志是MySQL提供的一种日志记录，它用来记录在MySQL中相应事件超过阈值的语句，具体指运行时间超过 long_query_time 值的SQL，则会被记录到慢查询日志中。
- long_query_time 的默认值为10，意思是运行时间超过10s以上的语句。
- 由他来查看哪些SQL超过了我们的最大忍耐时间，比如一条SQL执行超过了5秒钟，我们就算慢SQL，希望能收集超过5秒的SQL，结合之前的explain进行全面分析。

##### 怎么用

- 默认情况下，MySQL数据库没有开启慢日志查询，需要手动设置参数，如果不是调优需要，不建议启动该参数，因为会带来一定的性能影响
	- 查看：show variables like '%slow_query_log%’
		- 第二个内容就是慢查询日志的文件路径
	- 开启：set global slow_query_log = 1;
	- 设置慢的阈值时间：set global slow_query_time = 3;（重开会话窗口才能看到变化）
	- 查询当前系统中有多少条慢查询记录：show global status like ‘%Slow_queries%’;

##### 日志分析工具mysqldumpslow

使用mysqldumpslow --help查看帮助文档

### 3. 批量数据脚本

- 建表

- 设置参数 log_bin_trust_function_creators

	show variables like ‘log_bin_trust_function_creators’;

	set global log_bin_trust_function_creators = 1;

- 创建函数，保证每条数据都不同

- 创建存储过程

- 调用存储过程

### 4. Show Profile

1. 是否支持	show variables like ‘profiling’;
2. 开启功能，默认是关闭	set profiling=on;
3. 运行SQL
4. 查看结果，show profiles;
5. 诊断SQL，show profile cpu, block io for query 数字号码;
6. 四个耗时操作
	1. converting HEAP to MyISAM	查询结果太大，内存不够用了往磁盘上搬
	2. Creating tmp table	创建临时表
	3. Copying to tmp table on disk	把内存中临时表复制到磁盘，危险
	4. locked

### 5. 全局查询日志

1. 配置启用

	在mysql中的my.cnf中，设置如下

	```bash
	#开启
	general_log=1
	#记录日志文件的路径
	general_log_file=/path/logfile
	#输出格式
	log_output=FILE
	```

2. 编码启用

	```mysql
	set global general_log=1;
	set global log_output='TABLE';
	# 你所编写的sal语句将会记录在mysql库里的general_log表，可以用下面的命令查看
	select * from mysql.general_log;
	```

永远不要在生产环境开启这个功能。

## 六、MySQL锁机制

### 1. 表锁（偏读）

##### 特点

偏向MyISAM存储引擎，开销小，加锁快；无死锁；锁定粒度大，发生锁冲突的概率最高，并发度最低。

```mysql
# 查看表上加过的锁
show open tables;

# 手动增加表锁
lock table 表名字 read(write), 表名字2 read(write), 其它;

# 释放锁
unlock tables;

# 分析表锁定
show status like 'table%';
/*
这里有两个状态变量记录MySQL内部表级锁定的情况，两个变量说明如下：
Table_locks_immediate：产生表级锁定的次数，表示可以立即获取锁的查询次数，每立即获取锁值+1
Table_locks_waited：出现表级锁定征用而发生等待的次数（不能立即获取锁的次数，每等待一次值+1），此值高说明存在着较严重的表级锁定争用情况
*/
```

##### 场景1：session1对表A加==读锁==

- session1可以查询该表记录，其他session也可以查询该表记录

- session1不可以查询其他没有锁定的表，其他session可以查询或者更新未锁定的表

- session1插入或修改当前表都会报错，其他session插入或修改当前表会阻塞等待session1释放锁

##### 场景2：session1对表A加==写锁==

- session1对该表的查询+更新+插入都可以进行，其他session对表的查询会被阻塞

##### 总结

1. 对MyISAM表的读操作（加读锁），不会阻塞其他进程对同一表的读请求，但会阻塞对同一表的写请求，只有当读锁释放后，才会执行其他进程的写操作；
2. 对MyISAM表的写操作（加写锁），会阻塞其他进程对同一表的读和写操作，只有当写锁释放后，才会执行其他进程的读写操作。

此外，MyISAM的读写锁调度是写优先，这也是MyISAM不适合做写为主的引擎。因为写锁后，其他线程不能做任何操作，大量的更新会使查询很难得到锁，从而造成永远阻塞

### 2. 行锁（偏写）

##### 特点

偏向InnoDB存储引擎，开销大，加锁慢；会出现死锁；锁定粒度最小，发生锁冲突的概率最低，并发度最高。

InnoDB与MyISAM的最大不同有两点：一是支持事务；二是采用了行级锁。

##### 事务

mysql中，事务其实是一个最小的不可分割的工作单元，事务能够保证一个业务的完整性。事务通常有`四个属性`ACID：

- ==原子性==：事务是最小的单位，不可以再分割。
- ==一致性==：事务要求，同一事务中的 sql 语句，必须保证同时成功或者同时失败。
- ==隔离性==：数据库系统提供一定的隔离机制，保证事务在不受并发操作影响的“独立”环境执行。这意味着事务处理过程中的中间状态对外部是不可见的，反之亦然。
- ==持久性==：事务一旦结束，就不可返回，对于数据的修改是永久性的。

并发事务处理带来的问题：

- ==更新丢失==：当两个或多个事务选择同一行，然后基于最初选定的值更新该行时，由于每个事务都不知道其他事务的存在，就会发生更新丢失问题--最后更新覆盖了其他事务所做的更新。

- ==脏读==：一个事务正在对一条记录做修改，在这个事务在完成并提交前，这条记录的数据就处在不一致状态；这时另一事务也来读取同一条记录，如果不加控制，第二个事务读取了这些“脏”数据，并据此做进一步的处理，就会产生未提交的数据依赖关系。

	一句话：事务A读取了事务B**已修改但尚未提交**的数据，还在这个数据的基础上做了操作。此时事务B回滚，A读取的数据无效，不符合一致性要求 。

- ==不可重复读==：一个事务在读取某些数据后，再次读取以前读过的数据，却发现其读出的数据已经发生了改变、或某些记录已经被删除了。

	一句话： 事务A读取了事务B已经提交修改的数据，不符合隔离性。

- ==幻读==：一个事务按相同的查询条件重新读取以前检索过的数据，却发现其他事务插入了满足其查询条件的新数据。

	一句话：事务A读取到了事务B提交的新增数据，不符合隔离性。

	例子：如果a，b两个事务都在进行操作，如果 a事务 开启之后，他插入一条数据，b事务并不能看到a事务插入的数据，此时b事务也插入数据，就会有可能造成主键冲突，但是b事务查表又看不到冲突的数据。

	脏读和幻读有点相似，**脏读是事务B里面修改了数据**，**幻读是事务B新增了数据**。

##### 事务隔离级别

- 脏读、幻读、不可重复读，其实都是数据库读一致性问题，必须有数据库提供一定的事务隔离级别来解决。

| 级别     | 读数据一致性                             | 脏读 | 不可重复读 | 幻读 |
| -------- | ---------------------------------------- | ---- | ---------- | ---- |
| 未提交读 | 最低级别，只能保证不读取物理上损坏的数据 | Y    | Y          | Y    |
| 已提交读 | 语句级                                   | N    | Y          | Y    |
| 可重复读 | 事务级                                   | N    | N          | Y    |
| 可序列化 | 最高级别，事务级                         | N    | N          | N    |

数据库的事务隔离级别越严格，并发副作用越小，但付出的代价越大，因为事务隔离实质上就是使事务在意的那个程度上“串行化”进行，这显然与“并发”相矛盾。

多个事务对同一行的操作需要前一个事务对该行的操作已提交，否则该事务会被阻塞。如果两个事务操作的行不一样，那么不会造成阻塞。

##### 无索引导致行锁变表锁

比如第一个事务修改行时varchar类型不写单引号，导致索引失效，此时变为表锁，那么虽然第二个事务操作的行与第一个事务操作的行不一样，但也会阻塞等待，因为行锁变成了表锁。

##### 间隙锁

**什么是间隙锁**

当我们用范围条件而不是相等条件检索数据，并请求共享或排他锁时，InnoDB会给符合条件的已有数据记录的索引项加锁；对于键值在条件范围内但并不存在的记录，叫做  “ 间隙 ”。

InnoDB也会对这个 “ 间隙 ” 加锁，这种锁机制叫做 “ 间隙锁 ”。 

**危害**

因为Query执行过程中通过范围查找的话，他会锁定整个范围内的所有索引键值，即使这个键值在表中不存在。间隙锁有一个比较致命的缺点，就是当锁定一个范围键值之后，即使某些键值在表中并不存在，也无法插入数据，在某些场景下可能会对性能造成很大的危害。

**如何锁定某一行**

select xxx... for update锁定某一行后，其他的操作会被阻塞，直到锁定行的会话提交commit

**优化建议**

- 尽可能让所有数据检索都通过索引来完成，避免无索引行锁升级为表锁
- 合理设计索引，尽量缩小锁范围
- 尽可能较少检索条件，避免间隙锁
- 尽量控制事务大小，减少锁定资源量和时间长度
- 尽可能低级别事务隔离

### 3. 页锁

开销和加锁时间界于表锁和行锁之间；会出现死锁；锁定粒度界于表锁和行锁之间，并发度一般

## 七、主从复制

### 1. 复制的基本原理

slave会从master读取binlog来进行数据同步

MySQL复制过程分为三步：

1. master将改变记录到二进制日志（binary log）。这些记录过程叫做二进制日志事件，binary log events；
2. slave将master的binary log events拷贝到它的中继日志（relay log）
3. slave重做中继日志中的时间，将改变应用到自己的数据库中。MySQL复制是异步的且串行化。

### 2. 复制的基本原则

- 每个slave只有一个master

- 每个slave只能有一个唯一的服务器ID

- 每个master可以有多个slave

### 3. 复制的最大问题

网络上的延时

### 4. 一主一从常见配置

MySQL版本一致且后台以服务运行

主从都配置在[mysqld]结点下，都是小写

主机修改my.ini配置文件

从机修改my.cnf配置文件



























