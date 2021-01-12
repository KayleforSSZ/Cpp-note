# STL
> * [vector如何扩展内存和释放内存](#vector如何扩展内存和释放内存)
> * [STL中各种容器对比](#各种容器对比)
> * [STL中的swap函数](#STL中的swap函数)
> * [STL中哈希表扩容](#STL中的哈希表扩容)
> * [STL迭代器失效的情况和原因](#STL迭代器失效的情况和原因)
> * [vector删除元素后如何避免当前迭代器会失效](#vector删除元素后如何避免当前迭代器会失效)
> * [vector的iterator和const_iterator和const iterator](#vector的iterator和const_iterator和constiterator)

## [vector如何扩展内存和释放内存](https://www.cnblogs.com/biyeymyhjob/archive/2012/09/12/2674004.html)
> * 内存增长
>
> * [1.5还是2倍扩容](https://blog.csdn.net/yangshiziping/article/details/52550291)
>
> 	gcc 二倍扩容，VS2015 1.5倍扩容
>
> * 内存释放

## size_type、size_t、int

`size_type`

STL容器中的一个成员变量，是一种用以保存不同容器的任意大小的类型，它与size_t一样，目的都是为了使用起来与具体机器无关。标准库类型将size_type定义为unsigned类型，比如string类的string::size_type 就是一个代表string抽象长度的无符号整型。也就是说，string::size_type从本质上来说，是一个整型数。关键是由于机器的环境，它的长度有可能不同。

`size_t`

size_t是一些C/C++标准在stddef.h中定义的。这个类型足以用来表示对象的大小。size_t的真实类型与操作系统有关。

在32位机器中被普遍定义为：

typedef  unsigned int size_t;

而在64位机器中被定义为：

typedef  unsigned long size_t;

size_t在32位架构上是4字节，在64位架构上是8字节，在不同架构上进行编译时需要注意这个问题。而int在不同架构下都是4字节，与size_t不同；且int为带符号数，size_t为无符号数。

**为什么有时候不用int，而是用size_type或者size_t:**

与int固定四个字节不同有所不同,size_t的取值范围是目标平台下最大可能的数组尺寸,一些平台下size_t的范围小于int的正数范围,又或者大于unsigned int. 使用Int既有可能浪费，又有可能范围不够大。

### 注意：

```c++
string str="123";
int a=2;
cout<<a-str.size()<<endl;
```

这段代码的输出并不是-1，在我64位win10系统中的输出是：4294967295，这是因为str.size()返回值是size_type一个无符号整数，而编译器在int与size_type做减法时，都视为了无符号整数。

- 我们在使用 string::find的函数的时候，它返回的类型就是 string::size_type类型。而当find找不到所要找的字符的时候，它返回的是 npos的值，这个值是与size_type相关的。假如，你是用 string s; int rc = s.find(.....); 然后判断，if ( rc == string::npos ) 这样在不同的机器平台上表现就不一样了。如果，你的平台的string::size_type的长度正好和int相匹配，那么这个判断会侥幸正确。但换成另外的平台，有可能 string::size_type的类型是64位长度的，那么判断就完全不正确了。 所以，正确的应该是： string::size_type rc = s.find(.....); 这个时候使用 if ( rc == string::npos )就回正确了。

## STL是线程安全的吗

- 多个线程可以同时读取一个容器中的内容。 即此时多个线程调用 容器的不涉及到写的接口都可以。
- 对不同容器的多个写入者是安全的。即多个线程对不同容器的同时写入合法。但不能同时又读又写同一容器对象的。

## 各种容器对比

|容器	|底层数据结构|时间复杂度	|有无序	|可不可重复 |	其他|
| :------: | :------: | :------: |:------: |:------: |:------: |
|array		|数组			|	随机读改 O(1)| 无序	|可重复	|支持快速随机访问                                                         |
|vector		|数组			|	随机读改、尾部插入、尾部删除 O(1)、头部插入、头部删除 O(n)	|无序	|可重复	|支持快速随机访问                                                         |
|list		|双向链表			|插入、删除 O(1)、随机读改 O(n)								|无序	|可重复	|支持快速增删                                                            |
|deque		|双端队列			|头尾插入、头尾删除 O(1)									|无序	|可重复	|一个中央控制器 + 多个缓冲区，支持首尾快速增删，支持随机访问                    |
|stack		|deque / list	|顶部插入、顶部删除 O(1)									|无序	|可重复	|deque 或 list 封闭头端开口，不用 vector 的原因应该是容量大小有限制，扩容耗时   |
|queue		|deque / list	|尾部插入、头部删除 O(1)									|无序	|可重复	|deque 或 list 封闭头端开口，不用 vector 的原因应该是容量大小有限制，扩容耗时  	|
|priority_queue			|vector + max - heap	|插入、删除 O(log2n)				|有序	|可重复	|vector容器 + heap处理规则                                               |
|set		|红黑树			|插入、删除、查找 O(log2n)									|有序	|不可重复																		|
|multiset	|红黑树			|插入、删除、查找 O(log2n)									|有序	|可重复  |																		 |
|map		|红黑树			|插入、删除、查找 O(log2n)									|有序	|不可重复|																		|
|multimap	|红黑树			|插入、删除、查找 O(log2n)									|有序	|可重复  |	 |

## 数组和vector如何互相转换





## [STL中的swap函数](https://blog.csdn.net/ryfdizuo/article/details/6435847)

* 除了数组，其他容器在交换后本质上是将内存地址进行了交换，而元素本身在内存中的位置是没有变化
* swap在交换的时候并不是完全将2个容器的元素互换，而是交换了2个容器内的内存地址。

## STL中的哈希表扩容
* 这里需要知道STL中的swap底层，其实扩容也是vector扩容
    * 创建一个新桶，该桶是原来桶两倍大最接近的质数(判断n是不是质数的方法：用n除2到sqrt(n)范围内的数) ；
    * 将原来桶里的数通过指针的转换，插入到新桶中(注意STL这里做的很精细，没有直接将数据从旧桶遍历拷贝数据插入到新桶，而是通过指针转换两个桶的地址)
    * 通过swap函数将新桶和旧桶交换，销毁新桶

## 影响哈希表性能的因素

- 散列函数，一个好的散列函数的值应尽可能平均分布。

- 处理冲突方法。

- 负载因子的大小。太大不一定就好，而且浪费空间严重，负载因子和散列函数是联动的。

## map和unordered_map的区别

- map是按照operator<比较判断元素是否相同，以及比较元素的大小，然后选择合适的位置插入到树中。所以，如果对map进行遍历（中序遍历）的话，输出的结果是有序的。顺序就是按照operator< 定义的大小排序。
- map 的key需要定义operator< 。 而unordered_map需要定义hash_value函数并且重载operator ==。
- unordered_map比map要快一些

## map自定义键的方式

### 1、重载operator<

```c++
class Person {
public:
  int age;
public:
    Person(int _age = 10) {
        age = _age;
    }
    bool operator<(const Person& p) const {
        return age < p.age;
    }
};

int main(int argc, char** argv) {
    map<Person, int> mp;
    Person p1;
    Person p2(20);
    mp[p1]  = 1;
    mp[p2] = 2;
    return 0;
}
```

### 2、function

利用function对普通的比较函数进行封装

```c++
class Person {
public:
  int age;
public:
    Person(int _age = 10) {
        age = _age;
    }
};
// 普通函数
bool cmp(const Person& p1, const Person& p2) {
    return p1.age < p2.age;
}

int main(int argc, char** argv) {
    map<Person, int, function<bool(const Person&, const Person&)>> mp(cmp); // function进行封装
    Person p1;
    Person p2(20);
    mp[p1]  = 1;
    mp[p2] = 2;
    return 0;
}
```

### 3、重载operator()的类

将比较函数打包成可以直接调用的类

```c++
#include <bits/stdc++.h>
using namespace std;

class Person {
public:
  int age;
public:
    Person(int _age = 10) {
        age = _age;
    }
};

struct Compare
{
    bool operator()(const Person& p1, const Person& p2) const {
        return p1.age < p2.age;
    }
};

int main(int argc, char** argv) {
    map<Person, int, Compare> mp;
    Person p1;
    Person p2(20);
    mp[p1]  = 1;
    mp[p2] = 2;
    return 0;
}
```

### 4、less函数的模板定制

前面几种方法，无论我们怎么定义，在声明group的时候都需要指定第3个参数，有什么方法是需要指定的呢？当然有啦。

通过map的定义可知，第三个参数的默认值是less<key>。显而易见，less<key>属于模板类。那么，我们可以对它进行模板定制，如下所示。

```c++
#include <bits/stdc++.h>
using namespace std;

class Person {
public:
  int age;
public:
    Person(int _age = 10) {
        age = _age;
    }
};

template<>
struct std::less<Person>
{
public:
    bool operator()(const Person& p1, const Person& p2) const {
        return p1.age < p2.age;
    }
};

int main(int argc, char** argv) {
    map<Person, int> mp;
    Person p1;
    Person p2(20);
    mp[p1]  = 1;
    mp[p2] = 2;
    return 0;
}
```

## STL中的迭代器是什么

- 迭代器是一种抽象的设计概念，是一种行为类似于指针的对象，指针最常见的行为就是解引用和成员访问。因此迭代器最重要的工作就是重载operator*和operator->。迭代器使得算法独立于使用的容器类型，使我们能够依序访问某个容器所含的各个元素，而又无需暴露该容器的内部表述方式。

- 迭代器使用==类模板==来实现。

`迭代器相应型别`：迭代器所指之物的型别。可以通过函数模板的参数推导机制来实现。

```c++
template <class I, class T>
void func_imp1(I iter, T t)
{
    T tmp;	// 这里T就是所指之物的型别，可用于变量声明之 用
    // ...
};

template <class I>
inline
void func(I iter)
{
    func_imp1(iter, *iter);	// func的全部工作转移到func_imp1
}

int main() {
    int i;
    func(&i);
    return 0;
}
```

### Traits迭代器型别：

用于萃取迭代器的5种型别 value type、difference type、pointer、reference、iterator catagory

- ==value type==
	- 迭代器所指对象的型别

- ==difference type==
	- 两个迭代器之间的距离

- ==pointer type==
	- 一个pointer，指向迭代器所指之物

- ==reference type==
	- 函数如果想传回一个左值，都是以 by reference 的方式进行。
	- 如果 p 是一个mutable iterator时，如果其 value type 是 T，那么 *p 的型别应该是一个 T&。
	- 如果 p 是一个const iterator时，如果其 value type 是 T，那么 *p 的型别应该是一个 const T&。
	- 这里的 *p指的就是 reference type。

- ==iterator catagory==
	- **输入迭代器**：来自容器的信息被视为输入，对输入迭代器解引用将使程序能够读取容器中的值，但不一定能让程序修改值。
	- **输出迭代器**：将信息从程序传输给容器的迭代器，因此程序的输出就是容器的输入，解引用让程序能够修改容器值，而不能读取。
	- **正向迭代器**：只使用++运算符来遍历容器。
	- **双向迭代器**：可以++也可以--。
	- **随机访问迭代器**：具有双向迭代器的所有特性，同时添加了支持随机访问的操作。

### std::iterator

STL提供了一个 iterators class 如下，如果每个新设计的迭代器都继承自它，就可保证符合STL所需之规范。iterator class 不含任何成员，纯粹只是型别定义，所以继承它不会招致任何额外负担。

```c++
template <class Category,
		  class T,
		  class Distance = ptrdiff_t,
    	  class Pointer = T*,
		  class Reference = T&>
struct iterator {
    typedef Category  iterator_category;
    typedef T		  value_type;
    typedef Distance  difference_type;
    typedef Pointer   pointer;
    typedef Reference reference;
};
```

### 怎么设计一个迭代器

iterator设计：

- 首先定义5个迭代器类型，这些 classes 只作为标记用，所以不需要任何成员。用于针对不同的迭代器类型对某些函数进行重载。

	```c++
	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};
	```

- 定义一个迭代器基类

	```c++
	template <class Category,
			  class T,
			  class Distance = ptrdiff_t,
	    	  class Pointer = T*,
			  class Reference = T&>
	struct iterator {
	    typedef Category iterator_category;
	    typedef T		 value_type;
	    typedef Distance difference_type;
	    typedef Pointer pointer;
	    typedef Reference reference;
	};
	```

- 定义 iterator_traits 类

	```c++
	template <class Iterator>
	struct iterator_traits {
	    typedef typename Iterator::iterator_category iterator_category;
	    typedef typename Iterator::value_type 		 value_type;
	    typedef typename Iterator::difference_type 	 difference_type;
	    typedef typename Iterator::pointer			 pointer;
	    typedef typename Iterator::reference 		 reference;
	}
	```

- 针对原生指针 pointer 和 const-pointer 设计偏特化版的 iterator_traits 类

	```c++
	// pointer指针
	template <class Iterator>
	struct iterator_traits<T*> {
	    typedef typename random_access_iterator_tag  iterator_category;
	    typedef typename T					 		 value_type;
	    typedef typename ptrdiff_t 	 				 difference_type;
	    typedef typename T*							 pointer;
	    typedef typename T&							 reference;
	};
	// const-pointer指针
	template <class Iterator>
	struct iterator_traits<const T*> {
	    typedef typename random_access_iterator_tag  iterator_category;
	    typedef typename T					 		 value_type;
	    typedef typename ptrdiff_t 	 				 difference_type;
	    typedef typename T*							 pointer;
	    typedef typename T&							 reference;
	};
	```

- 这个模板函数可以很方便地决定某个迭代器的类型（category）

	```c++
	template <class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category
	iterator_category(const Iterator&){
	    typedef typename iterator_traits<Iterator>::iterator_category category;
	    return category();
	};
	```

- 这个模板函数可以很方便地决定某个迭代器的distance type

	```c++
	template <class Iterator>
	inline typename iterator_traits<Iterator>::difference_type*
	distance_type(const Iterator&){
	    return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	};
	```

- 这个模板函数可以很方便地决定某个迭代器的 value type

	```c++
	template <class Iterator>
	inline typename iterator_traits<Iterator>::value_type*
	value_type(const Iterator&){
	    return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	};
	```

## vector删除元素如何避免当前迭代器会失效
**迭代器失效就是指：迭代器指向的位置在该操作的前后不再相同。**删除时，将当前的迭代器存起来。

## STL迭代器失效的情况和原因
迭代器失效分三种情况考虑，也是分三种数据结构考虑，分别为数组型，链表型，树型数据结构。

* 数组型数据结构
	* 该数据结构的元素是分配在连续的内存中
	* insert和erase操作，都会使得删除点和插入点之后的元素挪位置，所以，插入点和删除掉之后的迭代器全部失效，也就是说insert(*iter)(或erase(*iter))，然后在iter++，是没有意义的。
	* 解决方法：erase(*iter)的返回值是下一个有效迭代器的值。 `iter = cont.erase(iter);`

```C++
//不要直接在循环条件中写++iter
for (iter = cont.begin(); iter != cont.end();)
{
   (*it)->doSomething();
   if (shouldDelete(*iter))
      iter = cont.erase(iter);  //erase删除元素，返回下一个迭代器
   else
      ++iter;
}
```

* 链表型数据结构
	* 对于list型的数据结构，使用了不连续分配的内存
	* 插入不会使得任何迭代器失效
	* 删除运算使指向删除位置的迭代器失效，但是不会失效其他迭代器.
	* 解决办法两种，erase(*iter)会返回下一个有效迭代器的值，或者erase(iter++).

* 树形数据结构
	* 使用红黑树来存储数据
	* 插入不会使得任何迭代器失效
	* 删除运算使指向删除位置的迭代器失效，但是不会失效其他迭代器.
	* **erase迭代器只是被删元素的迭代器失效，但是返回值为void**，所以要采用erase(iter++)的方式删除迭代器。

**注意** ：经过erase(iter)之后的迭代器完全失效，该迭代器iter不能参与任何运算，包括iter++,*ite

### 针对不同容器，具体情况如下：

#### 向容器添加元素

- 如果容器是vector或者string，且存储空间被重新分配，那么迭代器、指针和引用会失效。如果存储空间未重新分配，那么插入点之前的迭代器、指针和引用未失效，插入点之后的迭代器、指针和引用失效。
- 如果是deque，插入到首尾位置以外的任何位置都会导致迭代器、指针和引用失效。如果插入到首尾，迭代器会失效，但指向存在元素的引用或指针不会失效。
- 对于list和forward_list，指向容器的迭代器、指针和引用仍有效。

#### 从容器删除元素

- 对于list和forward_list，指向容器其他位置的迭代器、引用和指针仍有效
- 对于deque，如果在首尾之外的任何位置删除元素，那么指向被删除元素外其他元素的迭代器、引用和指针也会失效。如果是删除deque的尾元素，那么尾迭代器失效，其他迭代器、引用和指针不受影响。如果删除deque的首元素，那么首迭代器失效，其他迭代器、引用和指针不受影响。
- 对于vector和string，与插入操作迭代器失效与否是一样的



## vector的iterator和const_iterator和const iterator

* 三种的区别
    * iterator，可遍历，可改变所指元素 
    * const_iterator，可遍历，不可改变所指元素 
    * const iterator，不可遍历，可改变所指元素 
* const_iterator转iterator，iterator不能转const_iterator
    * const_iterator 主要是 **在容器被定义成常量、或者非常量容器但不想改变元素值的情况** 下使用的，而且容器被定义成常量之后，它返回的迭代器只能是const_iterator
    * 有些容器成员函数只接受iterator作为参数，而不是const_iterator。那么，如果你只有一const_iterator，而你要在它所指向的容器位置上插入新元素呢？
        * const_iterator转iterator
        * 强制转换的函数会报错，只能通过 `advance(a, distance(a, b));` 其中，distance用于取得两个迭代器之间的距离，advance用于将一个迭代器移动指定的距离
        * 如果a是iterator，b是const_iterator，distance会报错，可以显式的指明distance调用的模板参数类型，从而避免编译器自己得出它们的类型

```C++
typedef deque<int> IntDeque;
typedef IntDeque::iterator iter;
typedef IntDeque::const_iterator ConstIter;
IntDeque d;
ConstIter ci;
Iter i(d.begin());
advance(i,distance<ConstIter>(i,ci)); 
```

## STL的 sort 函数

sort 算法在数据量大的时候采用 **快排**，分段递归排序，一旦分段后的数据量小于某个门槛 16，改用 **插入排序**，，如果递归层次过深，还会改用 **堆排**

具体实现是这样的

```
sort(first, last);
```

首先计算 last - first 的长度，通过 **log(last - first) （2^k <= n 的最大 k ）** 计算最大递归深度 depth_limit，如果元素个数小于 16 ，执行**插入排序**；否则 判断是否到达最大递归深度，如果到达最大递归深度，执行**堆排**，否则进行**快排**。

在最后结束后，[first, last) 内会有很多“元素个数少于16”的子序列，每个子系列都有相当程度的排序，但尚未完全排序，此时再进行一次插入排序即可。

其中，快排采用三点中值方案来确定枢轴。

插入排序在数据量小时，不像其他复杂算法那样有着诸如递归调用等操作带来的额外负担，所以排序效果不错，且当序列接近有序时，插入排序效率也很不错。

## List的sort原理

- 在该排序算法的实现过程中，定义了一个类似于搬运作用的链表carry和具有中转站作用的链表counter，这里首先对counter[i]里面存储数据的规则进行分析；counter[i]里面最多存储数据个数为2^(i+1)，若存储数据超过该数字，则向相邻高位进位，即把counter[i]链表里的内容都合并到counter[i+1]链表。carry负责取出原始链表的头一个数据节点和交换数据中转站作用

- 思想是：从链表中取1个节点，放入counter[0]，再取1个节点，放入counter[0]，此时counter[0]装满了，会转移到counter[1]，然后counter[0]再存2个节点，转移到counter[1]，此时counter[1]满了。就转移到counter[2]，...，依此类推，合并几个counter，因为每一个counter里都是有序的，所以思想是==归并排序==。

## STL的allocator

1. 通常 C++ 的内存配置操作和释放操作是这样的

	```c++
	class Foo {...};
	Foo *pf = new Foo;	// 配置内存，然后构造对象
	delete pf;		    // 将对象析构，然后释放内存
	```

	 这其中的 new 包含两阶段操作：

	​		(1) 调用new配置内存，(2) 调用Foo:Foo() 构造对象内容；

	 delete 也包含两阶段操作：

	​		(1) 调用Foo:~Foo()将对象析构，(2) 调用 delete 释放内存

2. 为精密分工，STL allocator 将这两阶段操作区分开：

	- __内存配置__操作由 __alloc:allocate()__ 负责；
	- __内存释放__操作由 __alloc:deallocate()__ 负责；
	- __对象构造__操作由 __construct()__ 负责；
	- __对象析构__操作由 __destroy()__ 负责

3.  construct() 接受一个指针**p**和一个初值**value**，将初值设定到指针所指的空间上

4.  destroy() 有两个版本，第一个版本接受一个指针，然后将该指针所指之物析构

  第二个版本接受 first 和 last 两个迭代器，将 [ first, last ) 区间内的对象析构掉

<img src="图\destroy_construct.jpg" alt="destroy_construct" style="zoom: 25%;" />

5. SGI 设计了双层级内存配置器，第一级配置器直接使用malloc()和free()，第二级配置器则视情况采用不同的策略：

	1. 当配置区块超过128 bytes 时，视之为“足够大”，便调用第一级配置器；

		1. 第一级配置器的 allocate() 和 realloc() 都是在调用 malloc() 和 realloc() 不成功后，改调用 oom_malloc() 和 oom_realloc() 。后两者都有内循环，不断调用“内存不足处理例程”，期望在某次调用后，获得足够的内存而圆满完成任务。但如果 “内存不足处理例程” 并未被客端设定， oom_malloc() 和 oom_realloc() 便老实不客气地调用 __THROW_BAD_ALLOC，丢出 bad_alloc异常信息，或利用 exit(1)硬生生地中止程序。

	2. 当配置区块小于 128 bytes时，视之为“过小”，为了避免**太多小额区块造成内存的碎片**以及降低**额外负担**（比如分配一个很小的块，但系统需要内存去记录这个块的大小，那么这个块越小，因为记录的内存大小固定，所以额外负担所占比例愈大），便采用复杂的memory pool 整理方式，而不再求助于第一级配置器。

		<img src="图\alloc.jpg" alt="alloc" style="zoom:25%;" />

		1. 内存池管理，又叫次层配置：每次配置一大块内存，并维护对应之自由链表 free-list 。下次若再有相同大小的内存需求，就直接从 free-list 中拨出。如果客端释还小额区块，就由配置器回收到 free-list 中（配置器除了负责配置，还负责回收）。为了方便管理，SGI第二级配置器会主动将任何小额区块内存需求量上调至 8 的倍数（例如客端要求30bytes，就自动调整为 32 bytes），并维护 16 个 free-list，各自管理大小分别为8，16，24，32，40，48，56，64，72，80，88，96，104，112，120，128 bytes的小额区块。

		2. 首先判断区块大小，大于128 bytes 就调用第一级配置器，小于 128 bytes 就检查对应的 free-list，如果free-list有可用的区块，就直接拿来用，如果没有，就调用refill() 准备从内存池为free-list重新填充空间。

			<img src="图\freelist1.jpg" alt="freelist1" style="zoom:25%;" />

		3. 释放操作，首先判断区块大小，大于 128 bytes 就调用第一级配置器的 deallocate() 函数释放，小于 128 bytes 就找到对应的 free-list，将区块回收。

			<img src="图\freelist2.jpg" alt="freelist2" style="zoom:25%;" />

		4. 重新填充：当发现free-list中没有可用区块了时，就调用 refill() ，准备为 free-list 重新填充空间。新的空间将取自内存池。缺省取得20个新节点，但万一内存池空间不足，获得的节点数可能小于20。如果内存池空间不足，就调用配置heap空间，用来补充内存池，具体配置方式如下：

			<img src="图\memory.jpg" alt="memory" style="zoom:25%;" />























































