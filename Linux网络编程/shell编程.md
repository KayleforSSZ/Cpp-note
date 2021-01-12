## 一、shell脚本运行方法

```shell
1. chmod a+x myshell.sh
   ./myshell.sh
2. . myshell.sh
3. source myshell.sh
4. /bin/bash myshell.sh
```

`注意`：对于如下案例shell脚本

```shell
#!/bin/bash
cd ..
ls
```

- 对于==2和3==这两个运行方式，在执行完脚本后，终端的目录位置会是执行cd ..之后的位置（即返回上一级目录），而==1和4==这两个运行方式，执行完脚本后仍然处于当前目录（即运行前终端的目录位置）。

## 二、基本语法

### 1. 数据类型

shell脚本只有一种数据类型，就是`字符串`

### 2. 变量

#### 环境变量

- 环境变量可以从父进程传递给子进程，因此shell进程的环境变量可以从当前shell进程传递给fork出的子进程。用`printenv命令`可以显示当前shell进程的环境变量。

#### 本地变量

- 只存在于当前shell进程，用`set命令`可以显示当前shell进程中定义的所有变量（包括本地变量和环境变量）和函数
- 环境变量是任何进程都有的概念，而本地变量是shell特有的概念。在shell中，环境变量和本地变量的定义和用法相似。在shell中定义和赋值一个变量：

```
root@ssz:/home/ssz# VARNAME=value
```

​		注意等号两边都不能有空格，否则会被shell解释为命令和命令行参数

- 一个变量定义后仅存在于当前shell进程，它是本地变量，用`export命令`可以将本地变量导出为环境变量，定义和导出环境变量通常可以一步完成

```shell
root@ssz:/home/ssz# export VARNAME=value
```

​		也可以分两步完成

```shell
root@ssz:/home/ssz# VARNAME=value
root@ssz:/home/ssz# export VARNAME
```

- 用`unset命令`可以==删除==已定义的环境变量或本地变量

```shell
root@ssz:/home/ssz# unset VARNAME
```

### 3. 文件名代换（通配符）

- 这些用于匹配的字符称为通配符（wildcard），如：* ? [ ] 具体如下：

```shell
* 匹配0个或任意个字符
root@ssz:/home/ssz# ls *.sh
sample.sh

? 匹配一个任意字符
root@ssz:/home/ssz# ls myshel?.sh
sample.sh

[若干字符] 匹配方括号中任意一个字符的一次出现
root@ssz:/home/ssz# ls myshell.s[hijk]
sample.sh
```

### 4. 命令代换

- 由 “ `  ” 反引号括起来的也是一条==命令==，shell先执行该命令，然后将输出结果立刻代换到当前命令行中。例如定义一个变量存放 date 命令的输出：

```shell
root@ssz:/home/ssz# DATE=`date`
root@ssz:/home/ssz# echo $DATE
Fri Nov 20 14:29:09 CST 2020
```

- 命令代换也可以用 `$()` 表示

```shell
root@ssz:/home/ssz# DATE=$(date)
Fri Nov 20 14:29:09 CST 2020
```

### 5. 算术代换

- 使用 \$(()) ，用于算术计算，(()) 中的shell变量取值将转换为整数，**同样含义的 $[] 等价**，例如：

```shell
root@ssz:/home/ssz# VAR=45
root@ssz:/home/ssz# echo $(($VAR+3)) 等价于 $((VAR+3))、$[VAR+3]、$[$VAR+3]
48
```

- $(()) 中只能用+-*/和()运算符，并且只能做整数运算。

- $[base#n]，其中base表示进制，n按照base进制解释，后面再有运算数，按十进制解释

```shell
root@ssz:/home/ssz# echo $[2#10+11]		这里2#10表示2进制的10
13
root@ssz:/home/ssz# echo $[8#10+11]
19
root@ssz:/home/ssz# echo $[16#10+11]
27
```

#### $ 用法

- $变量名：取变量的值
- ${变量名}：取变量的值（更安全）
- $(命令)：取命令执行结果
- $((变量名))：对变量执行算术运算
- $[变量名]：对变量执行算术运算

### 6. 转义字符

- 和C语言类似，\ 在shell中被用作转义字符，用于去除紧跟其后的单个字符的特殊含义（回车除外），换句话说，紧跟其后的字符取字面值。例如：

```shell
root@ssz_webserver:/home/ssz# echo $SHELL
/bin/bash
root@ssz_webserver:/home/ssz# echo \$SHELL
$SHELL
root@ssz_webserver:/home/ssz# echo \\
\
```

- 比如创建一个文件名为“$ $”的文件（$中间有空格）可以这样：

```shell
root@ssz_webserver:/home/ssz# touch \$\ \$
```

- 还有一个字符虽然不具有特殊含义，但是要用它做文件名也很麻烦，就是-号。如果要创建一个文件名以-号开头的文件，这样是不正确的：

```shell
root@ssz_webserver:/home/ssz# touch -hello
touch: invalid option -- 'e'
Try 'touch --help' for more information.
```

- 即使加上\转义也还是报错：

```shell
root@ssz_webserver:/home/ssz# touch \-hello
touch: invalid option -- 'e'
Try 'touch --help' for more information.
```

- 因为各种UNIX命令都把-号开头的命令行参数当作命令的选项，而不会当做文件名。如果非要处理以-号开头的文件名，可以有两种方法：

```shell
root@ssz_webserver:/home/ssz# touch ./-hello
```

- 或者

```shell
root@ssz_webserver:/home/ssz# touch -- -hello
```

- `\还有一种用法，在\后敲回车表示续行`，shell并不会立刻执行命令，而是把光标移到下一行，给出一个续行提示符>，等待用户继续输入，最后把所有的续行接到一起当作一个命令执行。例如：

```shell
root@ssz_webserver:/home/ssz# ls \
> -l
total 4
-rwxrwxrwx 1 root root 24 Nov 19 21:27 sample.sh
```

### 7. 单引号

- 和C语言相同，shell脚本中的单引号和双引号一样都是`字符串的界定符`，而不是字符的界定符。`单引号用于保持引号内所有字符的字面值`，即使引号内的 \ 和回车也不例外，但是字符串中不能出现单引号，如果引号没有配对就输入回车，shell会给出续行提示符，要求用户把引号配对上。例如：

```shell
root@ssz_webserver:/home/ssz# echo '$SHELL'
$SHELL
root@ssz_webserver:/home/ssz# echo 'ABCD
> EFG'
ABCD
EFG
```

### 8. 双引号

- 被双引号括住的内容，将被视为单一字符串。它防止通配符扩展，但允许变量扩展。这点与单引号的处理方式不同。

```shell
root@ssz_webserver:/home/ssz# DATE=$(date)
root@ssz_webserver:/home/ssz# echo "$DATE"
Fri Nov 20 16:05:46 CST 2020
root@ssz_webserver:/home/ssz# echo '$DATE'
$DATE
```

- 再比如：

```shell
root@ssz_webserver:/home/ssz# VAR=200
root@ssz_webserver:/home/ssz# echo $VAR
200
root@ssz_webserver:/home/ssz# echo '$VAR'
$VAR
root@ssz_webserver:/home/ssz# echo "$VAR"
200
```

## 三、脚本语法

### 1. 条件测试

- `test命令`或者 `[ 命令`可以测试一个条件是否成立，如果测试结果为真，则该命令的exit status为0，如果测试结果为假，则命令的exit status为1（注意，与C语言的逻辑正好相反）。例如测试两个数的大小关系：

```shell
root@ssz_webserver:/home/ssz# var=2
root@ssz_webserver:/home/ssz# echo $var
2
root@ssz_webserver:/home/ssz# test $var -gt 1
root@ssz_webserver:/home/ssz# echo $?	查看上一条命令的执行结果
0
root@ssz_webserver:/home/ssz# test $var -gt 3
root@ssz_webserver:/home/ssz# echo $?
1
root@ssz_webserver:/home/ssz# [ $var -gt 3 ]
root@ssz_webserver:/home/ssz# echo $?
1
```

- 虽然看起来很奇怪，但左方括号 [ 确实是一个命令的名字，传给命令的各参数之间应该用空格隔开，比如：$var 、-gt 、3 、]  是 [ 命令的四个参数，它们之间必须用空格隔开。命令test或 [ 的参数形式是相同的，只不过test命令不需要 ] 参数。以 [ 命令为例，常见的测试命令如下表：

```shell
[ -d DIR ]	# 如果DIR存在并且是一个目录则为真
[ -f DILE ]	# 如果FILE存在且是一个普通文件则为真
[ -z STRING ]	# 如果STRING的长度为0则为真
[ -n STRING ]	# 如果STRIGN的长度非0则为真
[ STRIGN1 = STRING2 ]	# 如果两个字符串相同则为真
[ STRIGN1 != STRING2 ]	# 如果两个字符串不同则为真
[ ARG1 OP ARG2 ]	# ARG1和ARG2应该是整数或者取值为整数的变量，OP是 -eq(等于) -ne(不等于)
					# -lt(小于) -le(小于等于) -qt(大于) -qe(大于等于)之中的一个
```

- 和C语言类似，测试条件之间还可以做与、或、非逻辑运算

```shell
[ ! EXPR ]	# EXPR可以是上表中任意一种测试条件，!表示“逻辑非”
[ EXPR1 -a EXPR2 ]	# EXPR1和EXPR2可以是上表中任意一种测试条件，-a表示“逻辑与”
[ EXPR1 -o EXPR2 ]	# EXPR1和EXPR2可以是上表中任意一种测试条件，-o表示“逻辑或”
```

- 例如

```shell
root@ssz_webserver:/home/ssz# VAR=abc
root@ssz_webserver:/home/ssz# [ -d shell_file -a $VAR = 'abc' ]
root@ssz_webserver:/home/ssz# echo $?
0
```

- 注意，如果上例中的\$VAR变量事先没有定义，则被shell展开为`空字符串`，会造成测试条件的语法错误（展开为[ -d shell_file -a   = ‘abc’ ]），==作为一种好的shell编程习惯，应该总是把变量取值放在双引号之中==（展开为[ -d shell_file -a “”  = ‘abc’ ]）

```shell
root@ssz_webserver:/home/ssz# unset var
root@ssz_webserver:/home/ssz# [ -d Desktop -a $var = 'abc' ]
-bash: [: too many arguments
root@ssz_webserver:/home/ssz# [ -d Desktop -a "$var" = 'abc' ]
root@ssz_webserver:/home/ssz# echo $?
1
```

### 2. 分支

#### if / then / elif / else / fi

- 和C语言类似，在shell中用 if、then、elif、else、fi 这几条命令实现分支控制。这种流程控制语句本质上也是由若干条shell命令组成的，例如先前讲过的

```shell
if [ -f ~/.bashrc ]; then
	. ~/.bashrc
fi
```

- 其实是三条命令，if [ -f ~/.bashrc ] 是第一条，then . ~/.bashrc 是第二条，fi 是第三条。如果第二条命令写在同一行则需要用 ; 号隔开，一行只写一条命令就不需要写 ; 号了，另外，then 后面由换行，但这条命令没写完，shell会自动续行，把下一行接在 then 后面当作一条命令处理。和 [ 命令一样，要注意命令和各参数之间必须有空格。if 命令的参数组成一条子命令，如果该子命令的 exit status 为0（表示真），则执行 then 后面的子命令，如果 exit status非0（表示假），则执行 elif、else 或者 fi 后面的子命令。 if 后面的子命令通常是测试命令，但也可以是其他命令。shell脚本没有 {} 号，所以用 fi 表示 if 语句块的结束。见下例：

```shell
#! /bin/bash
if [ -f sample.sh ]
then 
	echo 'sample.sh is a file'
else
	echo 'sample.sh is NOT a file'
if:
then 
	echo 'always true'
fi
```

- “:” 是一个特殊的命令，称为`空命令`，该命令不做任何事，但 exit status 总是真。此外，也可以执行 /bin/true或者 /bin/false得到真或假的 exit status。在看一个例子：

```shell
#! /bin/bash
  
echo "Is it morning? Please answer yes or no."
read YES_OR_NO
if [ "$YES_OR_NO" = "yes" ]; then
        echo "Good morning!"
elif [ "$YES_OR_NO" = "no" ]; then
        echo "Good afternoon!"
else
        echo "Sorry, $YES_OR_NO not recognized. Enter yes or no."
        return ;
fi
```

- 上例的read命令的作用是等待用户输入一行字符串，将该字符串存到一个shell变量中。
- 此外，shell还提供了 && 和 || 语法，和C语言类似，具有 short-circuit 特性，很多shell脚本喜欢写成这样：

```shell
test "$(whoami)" != 'root' && (echo you are using a non-privileged account;)
```

- && 和 || 用于连接两个命令，而上面讲的-a和-o仅用于在测试表达式中连接两个测试条件，要注意它们的区别，例如：

```shell
test "$VAR" -gt 1 -a "$VAR" -lt 3
```

- 和以下写法是等价的

```shell
test "$VAR" -gt 1 && test "$VAR" -lt 3
```

#### case / esac

- case命令可类比C语言的 switch / case 语句，esac 表示 case 语句块的结束。C语言的 case 只能匹配整型或字符型常量表达式，而 shell 脚本的 case 可以匹配字符串和通配符，`每个匹配分支可以有若干条命令，末尾必须以;;结束`，执行时找到第一个匹配的分支并执行相应的命令，然后直接跳到esac之后，不需要像C语言一样用 break 跳出。

```shell
#! /bin/bash
  
echo "Is it morning? Please answer yes or no."
read YES_OR_NO
case "$YES_OR_NO" in
yes|y|Yes|YES)
        echo "Good morning!";;
[nN]*)
        echo "Good afternoon!";;
*)
        echo "Sorry, $YES_OR_NO not recognized. Enter yes or no."
        return 1;;
esac
```

### 3. 循环

#### for / do / done

- shell 脚本的 for 循环结构和 C 语言很不一样，它类似于某些编程语言的 foreach 循环。例如：

```shell
#! /bin/bash
for FRUIT in apple banana pear; do
	echo "I like $FRUIT"
done

#! /bin/bash
for TEST in `ls`; do
   ls -l $TEST
done

```

- FRUIT 是一个循环变量，第一次循环 $FRUIT 的取值是 apple ，第二次的取值是 banana，第三次的取值是 pear。再比如，要将当前目录下的 chap0、chap1、chap2等文件名改为chap0\~、chap1\~、chap2\~等（按照惯例，末尾有\~的文件名表示临时文件），这个命令可以这样写：

```shell
for FILENAME in chap?; do mv $FILENAME $FILENAME~; done
```

- 也可以这样写

```shell
for FILENAME in `ls chap?`; do mv $FILENAME $FILENAME~; done
```

#### while / do / done

- while 的用法和C语言类似。比如一个验证密码的脚本：

```shell
#! /bin/bash
echo "Enter password:"
read PASSWORD
while [ "$PASSWORD" != "secret" ]; do
	echo "Sorry, password error, please try again"
	read PASSWORD
done
```

- 下面的例子通过算数运算控制循环的次数

```shell
#! /bin/bash
COUNT=1
while [ "$COUNT" -lt 10 ]; do
	echo "Here we go again"
	COUNT=$(($CONUT+1))
done
```

- 另外，shell 还有 until 循环，类似 C 语言的 do ... while 。

#### break / continue

- break[n] 可以指定跳出几层循环；continue 跳过本次循环，但不会跳出循环。
- 即 break 跳出，continue 跳过

### 4. 位置参数和特殊变量

- 有很多特殊变量被 shell 自动赋值的，我们已经遇到了\$? 和\$1。其他的常用的位置参数和特殊变量在这里总结一下：

```shell
$0           // 相当于 C 语言 main 函数的 argv[0]
$1、$2...    // 这些称为位置参数，相当于 C 语言 main 函数的argv[1]、argv[2]...
$#			 // 相当于 C 语言 main 函数的 argc - 1，注意这里的 # 后面不表示注释
$@			 // 表示参数列表"$1" "$2"...，例如可以用在 for 循环中的 in 后面
$*			 // 表示参数列表"$1" "$2"...，例如可以用在 for 循环中的 in 后面
$?			 // 上一条命令的exut status
$$			 // 当前进程号
```

- 位置参数可以用 shift 命令左移。比如 shift 3 表示原来的 \$4 变成 \$1，原来的 \$5 变成 \$2 等等，原来的\$1、\$2、\$3 丢弃，\$0 不移动。不带参数的 shift 命令相当于 shift 1。例如：

```shell
#! /bin/bash

echo "The program $0 is now running"
echo "The first parameter is $1"
echo "The second parameter is $2"
echo "The third parameter is $3"
echo "The parameter list is $@"
shift
echo "The first parameter is $1"
echo "The second parameter is $2"
echo "The third parameter is $3"
echo "The parameter list is $@"
```

- 执行结果如下

```shell
root@ssz_webserver:/home/ssz# . ./argv.sh aa bb cc
The program ./argv.sh is now running
The first parameter is aa
The second parameter is bb
The third parameter is cc
The parameter list is aa bb cc
The first parameter is bb
The second parameter is cc
The third parameter is 
The parameter list is bb cc
```

### 5.  输入输出

#### echo

- 显示文本行或变量，或者把字符串输入到文件

```shell
echo [option] string
-e 解析转义字符
-n 不回车换行。默认情况echo回显得内容后面跟一个回车换行

echo "hello\n\n"  	// 输出字符串 "hello\n\n"
echo -e "hello\n\n"	// 输出字符串 "hello" 和两个空行

echo "hello"		// 输出字符串 "hello"，终端换行
echo -n "hello"		// 输出字符串 "hello"，终端不换行
```

#### 管道

- 可以通过 | 把一个命令得输出传递给另一个命令做输入

```shell
cat myfile | more		// 按页的形式读取文件
ls -l | grep "myfile"	
df -k | awk '{print $1}' | grep -v "文件系统"
df -k 查看磁盘空间，找到第一列（awk '{print $1}'），去除“文件系统”，并输出
```

#### tee

- `tee` 命令把结果输出到标准输出，同时输出到另一个相应的副本文件（标准输出一份，文件副本一份）

```shell
df -k | awk '{print $1}' | grep -v "文件系统" | tee a.txt
```

- tee -a a.txt表示追加操作

```shell
df -k | awk '{print $1}' | grep -v "文件系统" | tee -a a.txt
```

#### 文件重定向

- `cmd > file`	 把标准输出重定向到新文件中

```shell
root@ssz_webserver:/home/ssz# date > out
root@ssz_webserver:/home/ssz# cat out
Thu Dec  3 17:38:25 CST 2020
```

- `cmd >> file`	把标准输出追加到文件中

```shell
root@ssz_webserver:/home/ssz# ls | grep sample.sh >> out
root@ssz_webserver:/home/ssz# cat out 
Thu Dec  3 17:38:25 CST 2020
sample.sh
```

- `cmd > file 2>&1`	 标准输出和标准出错都重定向到新文件中

```shell
root@ssz_webserver:/home/ssz# rm as.txt > errfile 2>&1
root@ssz_webserver:/home/ssz# cat errfile 
rm: cannot remove 'as.txt': No such file or directory
```

- `cmd < file1 > file2`	从 file1 中读取到 file2

```shell
root@ssz_webserver:/home/ssz# cat < out > out1
root@ssz_webserver:/home/ssz# cat out1
Thu Dec  3 17:41:46 CST 2020
sample.sh
```

- `cmd < &fd`	将文件描述符 fd 作为标准输入

- `cmd > &fd` 	将文件描述符 fd 作为标准输出

- `cmd < &-`	关闭标准输入

### 6. 函数

- 和C语言类似，shell 中也有函数的概念，但是`函数定义`中==没有返回值==也==没有参数列表==。例如：

```shell
#! /bin/bash
foo(){ 
	echo "Function foo is called"
}
echo "-=start=-"
foo
echo "-=end=-"
```

- 注意函数体的左花括号 { 和后面的命令之间必须有空格或换行，如果将最后一条命令和右花括号 } 写在同一行，命令末尾必须有分号 ;。但，不建议将函数定义写在一行上，不利于脚本阅读。
- 在定义 foo() 函数时并不执行函数体中的命令，就像定义变量一样，只是给 foo 这个名一个定义，到后面调用 foo 函数的时候（注意 shell 中的函数调用不写括号）才执行函数体中的命令。shell 脚本中的函数必须先定义后调用，一般把函数定义语句写在脚本的前面，把函数调用和其他命令写在脚本的最后（类似 C 语言中的 main 函数，这才是整个脚本世纪开始执行命令的地方）。
- shell 函数没有参数列表并不表示不能传参数，事实上，函数就像是迷你脚本，调用函数时可以传任意个参数，在函数内同样是用 \$0、\$1、\$2等变量来提取参数，函数中的位置参数相当于函数的局部变量，改变这些变量并不会影响函数外面的 \$0、\$1、\$2等变量。函数中可以用 return 命令返回，如果 return 后面跟一个数字则表示函数的 exit status。

```shell
#! /bin/bash

foo(){
    echo "Function foo is called"
    for PARAM in $@; do
        echo "$PARAM"
    done
}
foo1(){
	echo $1
	echo $2
	echo $3
}
echo "-=start=-"
foo $@
foo1 $1 $2 $3
echo "-=end=-"
```

- 下面这个脚本可以`一次创建多个目录`，各目录通过命令行参数传入，脚本逐个测试各目录是否存在，如果目录不存在，首先打印信息然后试着创建该目录。

```shell
#! /bin/bash

is_directory() {
	DIR_NAME = $1
	if [ ! -d $DIR_NAME ]; then
		return 1
	else 
		return 0
	fi
}

for DIT in "$@"; do
	if is_directory "$DIR"
	then :
	else
		echo "$DIR doesn't exist. Creating it now..."
        mkdir $DIR > /dev/null 2>&1
        if [ $? -ne 0 ]; then
        	echo "Can't create directory $DIR"
        	exit 1
        fi
    fi
done
```





## 四、shell脚本调试方法

- shell 提供了一些用于调试脚本的选项，如：

	- `-n`	读一遍脚本中的命令但不执行，用于检查脚本中的语法错误。
	- `-v`	一边执行脚本，一边将执行过的脚本命令打印到标准错误输出
	- `-x`	提供跟踪执行信息，将执行的每一条命令和结果一次打印出来

- 这些选项有三种常见的使用方法

	1. 在命令行提供参数。如：

		`$ sh -x ./test.sh`

	2. 在脚本开头提供参数。如：

		`#！ /bin/bash -x`

	3. 在脚本中用 set 命令启用或者禁用参数

```shell
#! /bin/bash

if [ -z "$1" ]; then
	set -x
	echo "ERROR: Insufficient Args."
	exit 1
	set +x
fi
```

- set -x 和 set +x 分别表示启用和禁用 -x 参数，这样可以只对脚本中的某一段进行跟踪调试。

## 五、正则表达式

### 1. 基本语法

#### 字符匹配符

| 字符      | 含义                                                  | 举例                                             |
| --------- | ----------------------------------------------------- | ------------------------------------------------ |
| .         | 匹配任意一个字符                                      | abc. 可以匹配abcd、abc9等                        |
| [ ]       | 匹配括号中的任意一个字符                              | [abc]d可以匹配ad、bd或cd                         |
| -         | 在[ ]括号内表示字符范围                               | [0-9a-fA-F]可以匹配一位16进制数                  |
| ^         | 位于[ ]内的开头，匹配除括号中的字符之外的任意一个字符 | [^xy]匹配除xy之外的任一字符，如a1、b1            |
| [[:xxx:]] | grep工具预定义的一些命名字符类                        | [[:alpha:]]匹配一个字母，[[:digit:]]匹配一个数字 |

#### 数量限定符

|  字符  | 含义                               | 举例                                                         |
| :----: | :--------------------------------- | :----------------------------------------------------------- |
|   ?    | 紧跟在它前面的单元应匹配一次或零次 | [0-9]?\.[0-9]匹配0.0、2.3、.5等，由于 . 在正则表达式中是一个<br />特殊字符，所以需要用转义字符转义一下，取字面值 |
|   +    | 紧跟在它前面的单元应匹配一次或多次 | [a-zA-Z0-9\_.-]+@[a-zA-Z0-9\_.-]+\\.[a-zA-Z0-9_.-]+匹配      |
|   *    | 紧跟在它前面的单元应匹配零次或多次 | [0-9]\[0-9]*匹配至少一位数字，<br />等价于[0-9]+，[a-zA-Z_]+[a-zA-Z_0-9]\*匹配C语言的标识符 |
|  {N}   | 紧跟在它前面的单元应精确匹配N次    | [0-9]\[0-9]{2}匹配从100到999的整数                           |
| {N, }  | 紧跟在它前面的单元应匹配至少N次    | [0-9]\[0-9]{2, }匹配三位以上（含三位）的整数                 |
| {, N}  | 紧跟在它前面的单元应匹配最多N次    | [0-9]{,1}相当于[0-9]?                                        |
| {N, M} | 紧跟在它前面的单元应匹配N到M次     | [0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}匹配IP地址  |

- 再次注意 grep 找的是`包含某一模式`的行，而`不是完全匹配某一模式`的行

#### 位置限定符

| 字符 | 含义                       | 举例                                                  |
| ---- | -------------------------- | ----------------------------------------------------- |
| ^    | 匹配行首的位置             | ^Content 匹配位于一行开头的 Content                   |
| $    | 匹配行末的位置             | ;^匹配位于一行结尾的 ; 号，^$匹配空行                 |
| \\<  | 匹配单词开头的位置         | \\<th匹配 ... this。但不匹配 ethernet、tenth          |
| \\>  | 匹配单词结尾的位置         | p\\>匹配 leap ...，但不匹配 parent、sleepy            |
| \b   | 匹配单词开头或结尾的位置   | \\bat\\b 匹配 ... at ...，但不匹配 cat、atexit、batch |
| \B   | 匹配非单词开头或结尾的位置 | \Bat\B 匹配 battery，但不匹配 ... attend、hat ...     |

- 位置限定符可以帮助 grep 更准确的查找
- 例如，用 [0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3} 查找 ip 地址，找到这两行

```
192.168.1.1
1234.234.04.5678
```

- 如果用 ^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$ 查找，就可以把 1234.234.04.5678 这一行给过滤掉

- 也可以用 `^([0-9]{1,3}\.){3}[0-9]{1,3}$` 来查找 IP 地址

#### 其他特殊字符

| 字符 | 含义                                                         | 举例                                                       |
| ---- | ------------------------------------------------------------ | ---------------------------------------------------------- |
| \    | 转义字符，普通字符和特殊字符之间相互转义                     | 普通字符 < 写成 \\<表示单词开头的位置，特殊字符 . 写成 \\. |
| ()   | 将正则表达式一部分括起来组成一个单元，可以对整个单元使用数量限定符 | ^([0-9]{1,3}\\.){3}[0-9]{1,3}$ 匹配 IP 地址                |
| \|   | 连接两个子表达式，表示或的关系                               | n(o\|either) 匹配 no 或 neither                            |

#### Basic 正则和 Extended 正则区别

- 以上介绍的都是 grep 正则表达式的 Extendee 规范，Basic 规范也有这种语法，只是字符?+{}|()应解释为普通字符，要表示上述含义需要加 \ 转义。如果使用 grep 而不是 egrep，并且不加 -E 参数，则应该遵照 Basic 规范来书写表达式。

### 2. grep

#### 作用

- Linux 系统中 grep 命令是一种强大的文本搜索工具，它能使用正则表达式搜索文本，并把匹配的行打印出来。grep 全称是 Global Regular Expression Print，表示全局正则表达式版本，它的使用权限是所有用户。
- grep 家族包括 grep、egrep、fgrep。egrep 和 fgrep 的命令只跟 grep 有很小不同。egrep 是 grep 的扩展，支持更多的 re 元字符，fgrep 就是 fixed grep 或 fast grep，他们把所有的字母都看作单词，也就是说正则表达式中的元字符表示回其自身的字面意义，不再特殊。Linux 使用 GNU 版本的 grep。它功能强大，可以通过 -G、-E、-F 命令选项来使用 egrep 和 fgrep的功能。

#### 格式

```shell
grep [options]
主要参数：	grep --help 可查看
	-c：只输出匹配行的计数
	-i：不区分大小写
	-h：查询多文件时不显示文件名
	-l：查询多文件时只输出包含匹配字符的文件名
	-n：显示匹配行及行号
	-w：只匹配整个单词，而不是字符串的一部分
	-s：不显示不存在或无匹配文本的错误信息
	-v：显示不包含匹配文本的所有行
	--color=auto：可以将找到的关键词部分加上颜色的显示
```

- pattern 正则表达式主要参数：

```shell
\：忽略正则表达式中特殊字符的原有含义
^：匹配正则表达式的开始行
$：匹配正则表达式的结尾行
\<：从匹配正则表达式的行开始
\>：从匹配正则表达式的行结束
[ ]：单个字符，如[a]即a符合要求
[ - ]：范围，如[a-z]即a、b、c...z都满足
.：所有的单个字符
*：所有字符，长度可以为0
```

#### 实例

```shell
grep 'test' d*			显示所有以 d 开头的文件中包含test的行
grep 'test' aa bb cc 	显示在aa、bb、cc文件中匹配test的行
grep '[a-z]\{5\}' aa	显示所有包含每个字符串至少有5个连续小写字符的字符串的行
grep 'w\(es\)t.*\1' aa	如果west被匹配到，则es就被存储到内存中，并被标记为1，然后搜索任意个字符
						（.*），这些字符后面紧跟着另一个es（\1），找到就显示该行。如果用egrep或
						grep -E 就不用"\"号进行转义，直接写成 'w(es)t.*\1' 就可以了
```



### 3. find

- 由于 find 具有强大的功能，所以他的选项也很多，其中大部分选项都值得我们花时间来了解一下。即使系统中含有网络文件系统，find 命令在该文件系统中同样有效，只要你具有相应的权限。
- 在运行一个非常消耗资源的 find 命令时，很多人都倾向于把它放到后台执行，因为遍历一个大的文件系可能会花费很长的时间（这里是指30G字节以上的文件系统）

#### 格式

- find 命令的一般形式为

```shell
find pathname -options [-print -exec -ok ...]
```

- find 命令的参数
	- ==-name==  按名字搜索，`find ./ -name “init”`
	- ==-size==  按文件大小搜索，`find ./ -size +3M -size -7M` 
	- ==-type==  按文件类型搜索，-d/-f/-p/-l/-s/-c/-b，`find ./ -type f`
	- ==-maxdepth==  指定搜索递归深度，`find ./ -maxdepth 2  -type d`
	- ==-exec==  对匹配的文件执行该参数所给出的命令。`find ./ -maxdepth 2  -type d -exec ls -ld {} \;`
	- ==-ok==  和 -exec 的作用相同，只不过以一种更为安全的模式来执行，在执行前会询问，交互版的 -exec。`find -maxdepth 1 -type d -ok rm -rf {} \;`
	- ==-xargs==  对 find 查询的结果分批进行处理，结合管道使用，xargs是通过空格来区分查询的结果，进行处理的，所以如果文件名带有空格，就会失败。`find -maxdepth 1 -type f | xargs ls -ld`
	- ==-print==  将匹配的文件输出到标准输出，通过 print0 可以解决上述 xargs 的问题，因为这相当于对于查到的每一个文件默认在末尾加了个0，区分的时候就可以用0来区分。`find -maxdepth 1 -type f -print0 | xargs -0 ls -ld`
	- ==-atime|ctime|mtime==  天为单位，a：访问，c：文件内容修改，m：文件属性修改
	- ==-amin|cmin|mmin==  分钟为单位。`find ./ -name "syslog*" -mtime +5 -exec ls -l {} \;`

### 4. sed

- sed 意为流编辑器，在 shell 脚本和 makefile 中作为过滤器使用非常普遍，也就是把前一个程序的输出引入 sed 的输入，经过一系列编辑命令转换为另一种格式输出。sed 和 vi 都源于早期 UNIX 的 ed 工具，所以很多 sed 命令和 vi 的末行命令是相同的。
- sed 命令行的基本格式为

```shell
sed option 'script' file1 file2 ...			sed 参数 `脚本(/pattern/action)` 待处理文件
sed option -f scriptfile file1 file2 ...	sed 参数 -f `脚本文件` 待处理文件
```

- 选项含义：

```shell
--version					显示 sed 版本
--help						显示帮助文档
-n, --quiet, --silent		静默输出，默认情况下，sed 程序在所有的脚本指令执行完毕后，
							将自动打印模式空间中的内容，这些选项可以屏蔽自动打印
-e script					允许多个脚本指令被执行
-f script-file,			
--file=script-file			从文件中读取脚本指令，对编写自动脚本程序来说很棒
-i,--in-place				直接修改源文件，经过脚本指令处理后的内容将被输出至源文件
-l N, --line-length=N		该选项指定l指令可以输出的行长度，l指令用于输出非打印字符
--posix						禁用GNU sed扩展功能
-r, --regexp-extended		在脚本指令中使用扩展正则表达式
-s. --separate 				默认情况下，sed将把命令行指定的多个文件名作为一个长的连续的输入流
							而GNU sed则允许把他们当作单独的文件，这样如正则表达式则不进行跨文件匹配
```

- 以上仅是 sed 程序本身的选项功能说明，至于具体的脚本指令（即对文件内容做的操作）后面我们会详细描述，这里就简单介绍几个脚本指令操作作为 sed 程序的例子。

```shell
a, append			追加
i, insert			插入
d, delete			删除
s, substitution		替换
```

- 如：sed “s/para/##para/” ./file.sh 把 file.sh 中的 para 改成 ##para
- 如：sed “2a itcast” ./testfile 在输出 testfile 内容的第二行后添加 “itcast”

```shell
sed '2,5d' testfile
删除2-5行
```

- sed 处理的文件既可以由标准输入重定向得到，也可以当命令行参数传入，命令行参数可以一次传入多个文件，sed 会依次处理。sed 的编辑命令可以直接当命令行参数传入，也可以写成一个脚本文件然后用 -f 参数指定，编辑命令的格式为：

```shell
/pattern/action
```

- 其中 pattern 是正则表达式，action 是编辑操作。sed 程序一行一行读出待处理文件，如果某一行与 pattern 匹配，则执行相应的 action，如果一条命令没有 pattern 而只有 action。这个 action 将作用域待处理文件的每一行。


#### 常用sed命令

```shell
/pattern/p	打印匹配pattern的行
/pattern/d 	删除匹配pattern的行
/pattern/s/pattern1/pattern2/	查找符合pattern的行，将该行第一个匹配pattern1的字符串替换为pattern2
/pattern/s/pattern1/pattern2/g	查找符合pattern的行，将该行所有匹配pattern1的字符串替换为pattern2
```

- 使用 p 命令需要注意，sed 是把待处理文件的内容连同处理结果一起输出到标准输出的，因此 p 命令表示除了把文件内容打印出来之外还要额外打印一遍匹配 pattern 的行。比如一个文件 testfile 的内容是

```shell
123
abc
456
```

- 打印其中包含 abc 的行

```shell
root@ssz_webserver:/home/ssz# sed '/abc/p' testfile 
123
abc
abc
456
```

- 要想只输出处理结果，应加上 -n 选项，这种用法相当于 grep 命令

```shell
root@ssz_webserver:/home/ssz# sed -n '/abc/p' testfile 
abc
```

- 使用 d 命令就不需要 -n 选项了，比如删除含有 abc 的行

```shell
root@ssz_webserver:/home/ssz# sed '/abc/d' testfile 
123
456
```

- `注意`：sed 命令不会修改源文件，删除命令指标是某些行不打印输出，而不是从源文件中删去

- 使用查找替换命令时，可以把匹配 pattern1 的字符串复制到 pattern2 中，比如：

```shell
root@ssz_webserver:/home/ssz# sed 's/bc/-&-/' testfile
123
a-bc-
456
pattern2 中的 & 表示源文件的当前行中与 pattern1 相匹配的字符串
```

- 再比如

```shell
root@ssz_webserver:/home/ssz# sed 's/\([0-9]\)\([0-9]\)/-\1-~\2~/' testfile
-1-~2~3
abc
-4-~5~6
将第一个数前后加上- -，在第二个数前后加上 ~ ~，括号前面的 \ 是转义括号的
```

- pattern2 中的 \1 表示与 pattern1 的第一个 () 括号相匹配的内容，\2 表示与 pattern1 的第二个 () 括号相匹配的内容。sed 默认使用 Basic 正则表达式规范，如果指定了 -r 选项则使用 Extended 规范，那么 () 括号就不必转义了。如：

```shell
sed -r 's/([0-9])([0-9])/-\1-~\2~/' out.sh
```

- 替换结束后，所有行，含有连续数字的第一个数字前后都添加了 “-” 号；第二个数字前后都添加了 “~” 号。
- 可以一次指定多条不同的替换命令，用 “;” 隔开：

```shell
sed 's/yes/no/;s/static/dhcp/' testfile
注：使用分号;隔开指令
```

- 也可以使用 -e 选项来指定不同的替换命令，有几个替换命令需添加几个 -e 参数：

```shell
sed -e 's/yes/no/ -e s/static/dhcp/' testfile
注：使用 -e 选项
```

- 如果 testfile 的内容是

```html
<html><head><title>Hello World</title></head>
<body>Welcome to the world of regexp!</body></html>
```

- 现在要去掉所有的HTML标签，使输出结果为：

```html
Hello World
Welcome to the world of regexp!
```

- 怎么做呢？如果用下面的命令

```shell
sed 's/<.*>//g' testfile
```

- 结果是两个空行，把所有的字符都过滤掉了。这是因为，正则表达式中的数量限定符回匹配尽可能长的字符串，这称为贪心的。比如 sed 在处理第一行时，<.*>匹配的并不是<html>或<head>这样的标签，而是

```html
<html><head><title>Hello World</title></head>
```

- 这样一整行，因为这一行开头是<，中间是若干个任意字符，末尾是>。可以这样匹配

```shell
sed 's/<.[a-z]*>//g' testfile
或
sed 's/<[^>]*>//g' testfile
```

### 5. awk

- sed 是以行为单位处理文件，awk 比 sed 强的地方在于不仅能以行为单位还能以列为单位处理文件。`awk 缺省的行分隔符是换行，缺省的列分隔符是连续的空格和Tab`，但是行分隔符和列分隔符都可以自定义，比如 /etc/passwd 文件的每一行有若干个字段，字段之间以 : 分隔，就可以重定义 awk 的列分隔符为 : 并以列分隔符为单位处理这个文件。
- awk 实际上是一门很复杂的脚本语言，还有像 C 语言一样的分支和循环结构，但是基本用法和 sed 类似， awk 命令的基本形式为：

```shell
awk option 'script' file1 file2 ...			awk 参数 `脚本(pattern{action}` 待处理文件
awk option -f scriptfile file1 file2 ...	awk 参数 -f `脚本文件` 待处理文件
```

- 和 sed 一样，awk 处理的文件既可以由标准输入重定向得到，也可以当命令行参数传入，编辑命令可以直接当命令行参数传入，也可以由 -f 参数指定一个脚本文件，编辑命令的格式为：

```shell
/pattern/{actions}
condition{actions}
```

- 和 sed 类似，`pattern 是正则表达式，actions 是匹配成功后的一系列操作`。awk 程序一行一行读出待处理文件，如果某一行与 pattern 匹配，或者满足 condition 条件，则执行相应的 actions。`如果一条 awk 命令只有 actions 部分，则 actions 作用于待处理文件的每一行`。比如文件 testfile 的内容表示某商店的库存量：

```shell
ProductA 30
ProductB 76
ProductC 55
```

- 打印每一行的第二列

```shell
root@ssz_webserver:/home/ssz# awk '{print $2;}' testfile
30
76
55
```

- 自动变量 \$1、\$2 分别表示第一列、第二列等，类似于 Shell 脚本的位置参数，而 $0 表示整个当前行。再比如，如果某产品的库存量低于75则在行末标注需要订货：

```shell
awk '$2>75 {print $0} $2<75 {printf("%s %s\n", $0, "reorder");}' testfile
或
awk '$2>75 {print $0} $2<75 {printf "%s %s\n", $0, "reorder"}' testfile
或
awk '$2>75 {print $0} $2<75 {print $0 " reorder";}' testfile

ProductA 30     REORDER
ProductB 76
ProductC 55     REORDER
```

- 可见 awk 也有和 C 语言非常相似的 printf 函数。awk 命令的 condition 部分还可以是两个特殊的 condition-BEGIN 和 END，对于每个待处理文件，`BEGIN 后面的 actions 在处理整个文件之前执行一次，END 后面的 actions 在整个文件处理完之后执行一次。`
- awk 命令可以像 C 语言一样使用变量（但不需要定义变量），比如统计一个文件中的空行数

```shell
awk '/^ *$/ {x=x+1;} END {print x;}' testfile
```

- 再如，打印系统中的用户账号列表，也可以写作：

```shell
awk 'BEGIN {FS=":"} {print $1;}' /etc/passwd
```

- 就像 shell 的环境变量一样，有些 awk 变量是预定义的有特殊含义的，awk 常用的内建变量：

```shell
FILENAME	当前输入文件的文件名，该变量是只读的
NR			当前行的行号，该变量是只读的，R代表record
NF			当前行所拥有的列数，该变量是只读的，F代表field
OFS			输出格式的列分隔符，缺省是空格
FS			输入文件的列分隔符，缺省是连续的空格和Tab
ORS			输出格式的行分隔符，缺省是换行符
RS			输入文件的行分隔符，缺省是换行符
```

### 6. C程序中使用正则

- C语言处理正则表达式常用的函数有 regcomp()、regexec()、regfree() 和 regerror()，一般分为三个步骤，如下所示：

```shell
编译正则表达式 regcomp()
匹配正则表达式 regexec()
释放正则表达式 regfree()
```

- 这个函数把指定的正则表达式 pattern 编译成一种特定的数据格式 compiled，这样可以使匹配更有效。函数regexec() 会使用这个数据在目标文本串中进行模式匹配，执行成功返回0.

```c
int regcomp (regex_t *compiled, const char* pattern, int cflags)
    regex_t	 是一个结构体数据类型，用来存放编译后的正则表达式，它的成员 re_nsub 用来存储正则表达式中
    		 的子正则表达式的个数，子正则表达式就是用圆括号括起来的部分表达式
    pattern  是指向我们写好的正则表达式的指针
    cflags	 有如下4个值或者是它们或运算（|）后的值：
    		REG_EXTENDED	以功能更加强大的扩展正则表达式的方式进行匹配
    		REG_ICASE		匹配字母时忽略大小写
    		REG_NOSUB		不用存储匹配后的结果，只返回是否成功匹配，如果设置该标志位，那么在 regexec
    						nmatch = 0 和 pmatch = NULL，没有内嵌的子正则表达式
    		REG_NEWLINE		识别换行符，这样 '$' 就可以从行尾开始匹配，'^' 就可以从行的开头开始匹配
```

- 当我们编译好正则表达式后，就可以利用 regexec 匹配我们的目标文本串了，如果在编译正则表达式的时候没有指定 cflags 的参数为 REG_NEWLINE，则默认情况下是忽略换行符的，也就是把整个文本串当作一个字符串来处理。
- 执行成功返回0.失败返回错误码
- regmatch_t 是一个结构体数据类型，在 regex.h 中定义：

```c
typedef struct {
	regoff_t rm_so;
	regoff_t rm_eo;
}	regmatch_t;
```

- 成员 rm_so 存放匹配文本串在目标串中的开始位置，rm_eo 存放结束位置。通常我们以数组的形式定义一组这样的结构。因为往往我们的正则表达式中还包含子正则表达式。数组0单元存放主正则表达式的位置，后边的单元依次存放子正则表达式位置。

```c
int regexec (const regex_t *compiled, const char *string,
            	size_t nmatch, regmatch_t matchptr[], int eflags)
    compiled	是已经用 regcomp 函数编译好的正则表达式
    string 		是目标文本串
    nmatch		是 regmatch_t 结构体数组的长度，若使用该参数，需设置非 REG_NOSUB
    matchptr	regmatch_t 类型的结构体数组，存放匹配文本串的位置信息
    eflags 		有两个值：
    		REG_NOTBOL 让特殊字符 ^ 无作用
    		REG_NOTEOL 让特殊字符 $ 无作用
```

- 当我们使用完编译好的正则表达式后，或者要重新编译其他正则表达式的时候，我们可以用这个函数清空 compiled 指向的 regex_t 结构体的内容，请记住，如果是重新编译的话，一定要先清空 regex_t 结构体。

```c
void regfree (regex_t *compiled)
```

- 当执行 regcomp 或者 regexec 产生错误的时候，就可以调用这个函数而返回一个包含错误信息的字符串。

```c
size_t regerror(int errcode, regex_t *compiled, char *buffer, size_t length)
    errcode	 	是由 regcomp 和 regexec 函数返回的错误代码
    compiled	是已用 regcomp 函数编译好的正则表达式，这个值可以为 NULL
    buffer		指向用来存放错误信息的字符串的内存空间
    length		指明 buffer 的长度，如果这个错误信息的长度大于这个值，则 errcode 函数会自动截断
    			超出的字符串，但他仍然会返回完整的字符串的长度。所以我们可以用如下的方法先得到错误
    			字符串的长度
```

- 例如：size_t length = regerror(errcode, compiled, NULL, 0)
- 测试用例，输入一个正则表达式和一个字符串，判断能否匹配

```c
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Usage: %s RegexString Text\n", argv[0]);
        return 1;
    }
    
    const char * pregexstr = argv[1];	// 正则表达式
    const char * ptext = argv[2];		// 字符串
    regex_t oregex;						// 编译后的结构体
    int nerrcode = 0;
    char szerrmsg[1024] = {0};			// 保存错误信息的数组
    size_t unerrmsglen = 0;
    
    // 编译正则表达式，扩展正则
    if((nerrcode = regcomp(&oregex, pregexstr, REG_EXTENDED|REG_NOSUB)) == 0) {
        // 执行匹配，不保存匹配的返回值
		if((nerrcode = regexec(&oregex, ptext, 0, NULL, 0)) == 0) {
            printf("%s matches %s\n", ptext, pregexstr);
            regfree(&oregex);
            return 1;
        }
    }
    
    // 正则编译错误，szerrmsg中保存错误描述
    unerrmsglen = regerror(nerrcode, &oregex, szerrmsg, sizeof(szerrmsg));
    // 若错误信息较长
    unerrmsglen = unerrmsglen < sizeof(szerrmsg) ? unerrmsglen : sizeof(szerrmsg) - 1;
    
    szerrmsg[unerrmsglen] = '\0';
    printf("Regex error Msg: %s\n", szerrmsg);
    
    regfree(&oregex);
    return 0;
}
```

- 匹配网址：

```shell
./a.out "https:\/\/www\..*\.com" "https://www.taobao.com"
```

- 匹配邮箱

```shell
./a.out "^[a-zA-Z0-9]+@[a-zA-Z0-9]+.[a-zA-Z0-9]+" "itcast123@itcast.com"
./a.out "\w+([-+.]\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*" "itcast@qq.com"
注：\w 匹配一个字符，包含下划线
```

