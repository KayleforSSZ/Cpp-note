# C++补充

## 异常处理

对于如下代码

```c++
#include <bits/stdc++.h>
void foo(int m, int n) {
    int t = m/n;
}
int main(int argc, char* argv[]) {
    foo(1, argc-1);
    return 0;
}
```

运行后会提示 “ 出现未处理的异常 ” ，这是因为foo函数里未处理 n 为0 的情况，异常需要捕获。可以使用 try catch来捕获

```c++
#include <bits/stdc++.h>
using namespace std;

void foo(int m, int n)
{
	try
	{
		int t = m / n;
	}
	catch (...)	// ...表示接住任何异常
	{
		cout << "error" << endl;
	}
}
int main(int argc, char *argv[])
{
	foo(1, argc - 1);
	return 0;
}
```



## [构造函数可以抛出异常吗，有什么问题？](https://www.cnblogs.com/qinguoyi/p/10304882.html)

构造函数中应该避免抛出异常。
> * 构造函数中抛出异常后，对象的析构函数将不会被执行
> * 构造函数抛出异常时，本应该在析构函数中被delete的对象没有被delete，会导致内存泄露
> * 当对象发生部分构造时，已经构造完毕的子对象（非动态分配）将会逆序地被析构。

## 初始化列表的异常怎么捕获？
> * 初始化列表构造，当初始化列表出现异常时，程序还未进入函数体，因此函数体中的try-catch不能执行，catch也无法处理异常。可以通过函数try块解决该问题。
> * 函数try块中的try出现在表示构造函数初始值列表的冒号以及表示构造函数体的花括号之前，与这个try关联的catch既能处理构造函数体抛出的异常，也能处理成员初始化列表抛出的异常。


## 析构函数可以抛出异常吗，有什么问题？
析构函数不应该抛出异常
> * **其他正常，仅析构函数异常**。 如果析构函数抛出异常，则异常点之后的程序不会执行，如果析构函数在异常点之后执行了某些必要的动作比如释放某些资源，则这些动作不会执行，会造成诸如资源泄漏的问题。
> * **其他异常，且析构函数异常**。 通常异常发生时，c++的机制会调用已经构造对象的析构函数来释放资源，此时若析构函数本身也抛出异常，则前一个异常尚未处理，又有新的异常，会造成程序崩溃的问题。

## 析构函数如何处理异常？
> * 若析构函数抛出异常，调用std::abort()来终止程序
> * 在析构函数中catch捕获异常并作处理，吞下异常；
> * 如果客户需要对某个操作函数运行期间抛出的异常做出反应，class应该提供普通函数执行该操作，而非在析构函数中。

## shared_ptr是线程安全的吗

**1）从引用计数的角度来看：**
虽然引用计数存在于每一个`shared_ptr`对象中，但是实际上它是要跟随对象所管理的资源。引用计数会随着指向这块资源的`shared_ptr`对象的增加而增加。因此引用计数是要指向同一块资源的所有的对象共享的，所以实际上引用计数在`shared_ptr`底层中是以指针的形式实现的，所有的对象通过指针访问同一块空间，从而实现共享。

那么也就是说，引用计数是一个临界资源，所以在多线程中，我们必须要保证临界资源访问的安全性，因此在`shared_ptr`底层中在对引用计数进行访问之前，首先对其加锁，当访问完毕之后，在对其进行解锁。

**所以shared_ptr的引用计数是线程安全的。**

**（2）从被shared_ptr对象所管理的资源来看：**
`shared_ptr`对象所管理的资源存放在堆上，它可以由多个`shared_ptr`所访问，所以这也是一个临界资源。**因此当多个线程访问它时，会出现线程安全的问题**。

首先`shared_ptr`对象有两个变量，一个是指向的对象的指针，还有一个就是我们上面看到的引用计数， 当`shared_ptr`发生拷贝的时候，是先拷贝智能指针，然后再拷贝引用计数，也就是说，**shared_ptr的拷贝并不是一个原子操作**。而问题就出现在这里。

所以多个`shared_ptr`对象对其所管理的资源的访问不是线程安全的。如果不使用锁这会造成线程安全问题。

## [智能指针](https://www.cnblogs.com/TianFang/archive/2008/09/20/1294590.html)
智能指针有shared_ptr,weak_ptr,unique_ptr，[参考](https://www.cnblogs.com/wxquare/p/4759020.html)，使用普通指针，容易造成堆内存泄露（忘记释放），二次释放，程序发生异常时内存泄露等问题等，使用智能指针能更好的管理堆内存。

### auto_ptr

- auto_ptr指针在c++11标准中就被废除了，可以使用unique_ptr来替代，功能上是相同的，unique_ptr相比较auto_ptr而言，提升了安全性（没有浅拷贝），增加了特性（delete析构）和对数组的支持。

- **有两个不好的地方**
	- 不能有两个auto_ptr对象拥有同一个内部指针的所有权，因为有可能某一时刻，两者均尝试析构这个内部指针。

		```c++
		int* p = new int(3);
		// 这样是错的
		std::auto_ptr<int> aptr1(p);
		std::auto_ptr<int> aptr2(p);
		```

	- 当**两个auto_ptr对象**之间发生**赋值**操作时，内部指针被拥有的所有权会转移，这意味着右者对象会丧失所有权，不再指向这个内部指针（会被设置为NULL）

		```c++
		//可以用其他的auto_ptr指针进行初始化
		std::auto_ptr<int> aptr2 = aptr;
		printf("aptr2 %p : %d\r\n", aptr2.get(), *aptr2);
		```

- auto_ptr的构造的参数可以是一个指针，或者是另外一个auto_ptr对象。当一个新的auto_ptr获取了内部指针的所有权后，之前的拥有者会释放其所有权。

- auto_ptr.get()获取地址，*ptr获取内容

	```c++
	int main() {
	
		std::auto_ptr<int> aptr(new int(3));
		printf("auto_ptr %p : %d\r\n", aptr.get(), *aptr);
	
	    return 0;
	}
	/*
	auto_ptr 0000000000d71730 : 3
	*/
	```

- auto_ptr.release()解除auto_ptr与指针的关系，并不释放掉原来的指针

	```c++
	int main() {
		int* p = new int(4);
	    printf("p %p : %d\r\n", p, *p);
	    
		std::auto_ptr<int> aptr(p);
		printf("auto_ptr %p : %d\r\n", aptr.get(), *aptr);
	    
		int* ptr = aptr.release();
		printf("ptr %p : %d\r\n", ptr, *ptr);
	    return 0;
	}
	/*
	可以看出，原来指针的内容并没有被释放，只是解除了关系
	p 0000000001001730 : 4
	auto_ptr 0000000001001730 : 4
	ptr 0000000001001730 : 4
	*/
	```
	
- auto_ptr.reset(p)，将智能指针对象重置为p，并释放掉原来的指针

	```c++
	int main() {
		int* p1 = new int(3);
	    printf("p1 %p : %d\r\n", p1, *p1);
	    int* p2 = new int(4);
		std::auto_ptr<int> aptr(p1);
		printf("auto_ptr %p : %d\r\n", aptr.get(), *aptr);
	    
	    aptr.reset(p2);
	    printf("auto_ptr %p : %d\r\n", aptr.get(), *aptr);
		printf("p1 %p : %d\r\n", p1, *p1);
	    return 0;
	}
	/*
	p1 0000000000781730 : 3
	auto_ptr 0000000000f41730 : 3
	auto_ptr 0000000000f41750 : 4
	p1 0000000000781730 : 7871136
	*/
	```

- auto_ptr 存在的问题

	- **作为参数传递会存在问题**

		- 因为有拷贝构造和赋值的情况下，会释放原有的对象的内部指针，所以当有函数使用的是auto_ptr时，调用后会导致原来的内部指针释放。

		```c++
		void foo_test(std::auto_ptr<int> p){
			printf("%d\r\n", *p);
		}
		int main(){
			std::auto_ptr<int> p1 = std::auto_ptr<int>(new int(3));
			foo_test(p1);
			//这里的调用就会出错，因为拷贝构造函数的存在，p1实际上已经释放了其内部指针的所有权了
			printf("%d\r\n", *p1);
			return 0;
		}
		```

	- **不能使用vector数组**

### shared_ptr

* shared_ptr核心要理解引用计数，什么时候销毁底层指针，还有赋值，拷贝构造时候的引用计数的变化，析构的时候要判断底层指针的引用计数为0了才能真正释放底层指针的内存
   * 不能将指针直接**赋值**给一个智能指针，一个是类，一个是指针。例如`std::shared_ptr<int> p4 = new int(1);`
   * 可以`std::shared_ptr<int>p4(new int(1));`**初始化**
   * 拷贝使得对象的引用计数增加1，赋值使得原对象引用计数减1，当计数为0时，自动释放内存。后来指向的对象引用计数加1，指向后来的对象
   * 赋值操作符减少左操作数所指对象的引用计数（如果引用计数为减至0，则删除对象），并增加右操作数所指对象的引用计数
   
* shared_ptr创建后是栈上的对象，当出作用域后，每个对象会自动调用析构函数，如上所述，new int(1)会生成一个指针，此时将其传参数给shared_ptr,由shared_ptr对其进行管理，shared_ptr虽然是对象，但其有指针的特性，通过重载运算符*和->实现指针的特性来访问被管理的指针。

* shared_ptr是可以共享所有权的智能指针
    * shared_ptr的管理机制其实并不复杂，就是对所管理的对象（这里的对象本质是被管理的指针new int(1)，并不是类和对象中的对象）进行了引用计数，当新增一个shared_ptr对该对象进行管理时，就将该对象的引用计数加一；减少一个shared_ptr对该对象进行管理时，就将该对象的引用计数减一，如果该对象的引用计数为0的时候，说明没有任何指针对其管理，才调用delete释放其所占的内存。   
    * 对shared_ptr进行初始化时不能将一个普通指针直接赋值给智能指针，因为一个是指针，一个是类，可以通过make_shared函数或者通过构造函数传入普通指针
    * **不要把一个原生指针给多个shared_ptr**，不要把this指针交给智能指针管理，这样会重复释放
    * shared_ptr之间的资源共享是通过shared_ptr智能指针拷贝、赋值实现的，因为这样可以引起计数器的更新；__而如果直接通过原生指针来初始化，就会导致m_sp和p都根本不知道对方的存在，然而却两者都管理同一块地方__

```C++
int* ptr = new int;
shared_ptr<int> p1(ptr);
shared_ptr<int> p2(ptr); //这样不会导致更新，两者不知对方存在
shared_ptr<int> p3(p1);//这样才会导致计数器更新
```

* shared_ptr循环引用导致内存泄漏，引出weak_ptr
    * 循环引用是两个强引用（shared_ptr）互相引用，使得两者的引用计数无法为0，进而无法释放，此时将循环引用的一方变为weak_ptr即可。

```C++
#include <bits/stdc++.h>
using namespace std;

class CB;
class CA
{
public:
    CA(){
        cout << "CA() called! " << endl;
    }
    ~CA(){
        cout << "~CA() called! " << endl;
    }
    void set_ptr(shared_ptr<CB> &ptr){
        m_ptr_b = ptr;
    }
private:
    shared_ptr<CB> m_ptr_b;
};

class CB
{
public:
    CB(){
        cout << "CB() called! " << endl;
    }
    ~CB(){
        cout << "~CB() called! " << endl;
    }
    void set_ptr(shared_ptr<CA> &ptr){ 
        m_ptr_a = ptr; 
    }
private:
    shared_ptr<CA> m_ptr_a;
};

void test_refer_to_each_other()
{
    shared_ptr<CA> ptr_a(new CA());
    shared_ptr<CB> ptr_b(new CB());

    cout << "a use count : " << ptr_a.use_count() << endl;
    cout << "b use count : " << ptr_b.use_count() << endl;

    ptr_a->set_ptr(ptr_b);
    ptr_b->set_ptr(ptr_a);

    cout << "a use count : " << ptr_a.use_count() << endl;
    cout << "b use count : " << ptr_b.use_count() << endl;
}
int main()
{
    test_refer_to_each_other();
    return 0;
}
/*
CA() called! 
CB() called!
a use count : 1
b use count : 1
a use count : 2
b use count : 2
*/
```
通过结果可以看到，最后`CA`和`CB`的对象并没有被析构。

### weak_ptr


* **弱引用（weak_ptr）并不修改该对象的引用计数**，weak_ptr必须从一个share_ptr或另一个weak_ptr转换而来，这也说明，进行该对象的内存管理的是那个强引用的share_ptr,weak_ptr只是提供了对管理对象的一个访问手段这意味这弱引用它并不对对象的内存进行管理。__在功能上类似于普通指针，然而一个比较大的区别是，弱引用能检测到所管理的对象是否已经被释放，从而避免访问非法内存__

```C++
class CB
{
public:
    CB()
    {
        cout << "CB() called! " << endl;
    }
    ~CB()
    {
        cout << "~CB() called! " << endl;
    }
    void set_ptr(
        shared_ptr<CA> &ptr) { m_ptr_a = ptr; 
    }

private:
    weak_ptr<CA> m_ptr_a;
};
/*
CA() called! 
CB() called!
a use count : 1
b use count : 1
a use count : 2
b use count : 2
~CA() called! 
~CB() called!
*/
```
**expired()** 用于检测所管理的对象是否已经释放；

**lock()** 用于获取所管理的对象的强引用指针，不能直接访问弱引用，需要将其先通过lock转换为强引用再访问

#### 计数器增减的规则：

初始化及增加的情形

- 当创建一个新的shared_ptr时，内部对象计数器T和自身的计数器Ref均置1；
- 当将另外一个shared_ptr赋值给新的shared_ptr时，内部对象计数器+1，自身计数器不变；
- 当将另外一个shared_ptr赋值给新的weak_ptr时，内部对象计数器不变，自身计数器+1；
- 当从weak_ptr获取一个shared_ptr时，内部对象计数器+1，自身计数器不变。
- 规律就是当shared_ptr加1的时候，内部对象计数器+1；当weak_ptr加1的时候，自身计数器+1。

减少的情形：

- 当一个shared_ptr析构时，内部对象计数器-1.当内部对象计数器减为0时，则释放内部对象，并将自身计数器-1。
- 当一个weak_ptr析构时，自身计数器-1，当自身计数器减为0时，则释放自身_Ref_count*对象。

### unique_ptr

unique_ptr实现独占式拥有或严格拥有概念，保证同一时间内只有一个智能指针可以指向该对象

* unique_ptr禁止使用**拷贝构造**和**赋值运算符**

    ```c++
    unique_ptr<int> p(new int(5));
    // 这3种是不允许使用的
    unique_ptr<int> p2 = p;
    unique_ptr<int> p3(p);
    p4 = p;
    ```
    
    因此，这就从根源上杜绝了auto_ptr作为参数传递的写法了。
    
* unique_ptr多了move的用法。因为unique_ptr不能将自身对象内部指针直接赋值给其他unique_ptr，所以这里可以使用std::move()函数，让unique_ptr交出其内部指针的所有权，而自身置空，内部指针不会释放。

  ```c++
  void foo_move(){
      int* p = new int(3);
      std::unique_ptr<int> uptr(p);
      std::unique_ptr<int> uptr2 = std::move(uptr);
  }
  ```

- 数组

	```c++
	void foo_ary(){
	    std::vector<std::unique_ptr<int>> Ary;
	    std::unique_ptr<int> uptr(new int(3));
	    Ary.push_back(std::move(uptr));
	    printf("%d\r\n", *uptr);
	}
	```

## 自己实现一个智能指针

1. 用起来像指针
2. 会自己对资源进行释放

```c++
template<typename T>
class SmartPointer 
{
private:
    T* ptr;
    size_t* count;
public:
    // 初始化
    SmartPointer(T* _ptr = nullptr) : ptr(_ptr){
        if(ptr) {
            count = new size_t(1);
        }
        else {
            count = new size_t(0);
        }
    }
    // 拷贝构造
    SmartPointer(const SmartPointer& sptr) {
        if(this == &sptr) {
            return;
        }
        this->ptr = sptr.ptr;
        this->count = sptr.count;
        (*this->count)++;
    }
    // 赋值运算符
    SmartPointer& operator=(const SmartPointer& sptr) {
        if(this->ptr == sptr.ptr) {
            return *this;
        }
        // =左边所指对象引用计数减一
        if(this->ptr) {	
            (*this->count)--;
            if(this->count == 0) {
                delete this->ptr;
                delete this->count;
            }
        }
        this->ptr = sptr.ptr;
        this->count = sptr.count;
        (*this->count)++;
        return *this;
    }
    // 重载*
    T& operator*() {
        assert(this->ptr == nullptr);
        return *(this->ptr);
    }
    // 重载->
    T* operator->() {
        assert(this->ptr == nullptr);
        return this->ptr;
    }
    // 析构函数
    ~SmartPointer() {
        if(*this->count == 0) {
            delete this->ptr;
            delete this->count;
            std::cout << "释放" << endl;
        }
        else {
            (*this->count)--;
        }
        if(*this->count == 0) {
            delete this->ptr;
            delete this->count;
            std::cout << "释放" << endl;
        }
    }
    size_t use_count() {
        return *this->count;
    }
};
int main() {
    SmartPointer<int> sp1(new int(3));
    SmartPointer<int> sp2(sp1);
    SmartPointer<int> sp3(new int(3));
    sp2 = sp3;
    cout << sp1.use_count() << endl;
    cout << sp3.use_count() << endl;
    return 0;
}
```



## 内存泄漏
当一个对象已经不需要再使用本该被回收时，另外一个正在使用的对象持有它的引用从而导致它不能被回收，这导致本该被回收的对象不能被回收而停留在堆内存中，这就产生了内存泄漏。

### 内存泄漏检测方法

- VS有一个CRT函数	_CrtDumpMemoryLeaks()函数，头文件<crtdbg.h>，程序前面加上 #define CRTDBG_MAP_ALLOC

- Linux系统下内存泄漏的检测方法（valgrind）

	```c++
	g++ -g -o test test.cpp
	valgrind --tool=memcheck ./test
	```

	可以检测如下问题：

	- 使用未初始化的内存
	- 内存读写越界
	- 内存覆盖（strcpy/strcat/memcpy）
	- 动态内存管理
	- 内存泄漏

### 如何避免内存泄漏

+ 尽量避免在堆上分配内存
+ 不要手动管理内存，可以尝试在适用的情况下使用智能指针
+ 使用RAII
+ 在C++中避免内存泄漏的最好方法是尽可能少地在程序级别上进行new和delete调用--最好是没有

## 野指针
野指针指向一个已删除的对象或 申请访问受限内存区域的指针。

* 原因
  * 指针变量未初始化
  * 指针释放未置空
  * 指针操作超出作用域。返回指向栈内存的指针或引用，因为栈内存在函数结束时会被释放。

## 强制转换
C++中强制转换为static_cast, dynamic_cast,const_cast, reinterpret_cast，主要是为了解决C语言强制类型转换的以下三个缺点：

1. **没有从形式上体现出转换功能和风险的不同**

	例如，将int转换为double是没有风险的，但是将常量指针转换成非常量指针，将基类指针转换成派生类指针都是高风险的，而且后两者带来的风险不同，C语言的强制类型转换不对这些不同不加以区分。

2. **将多态基类指针转换成派生类指针时不检查安全性，即无法判断转换后的指针是否确实指向一个派生类对象**

3. **难以在程序中寻找到底什么地方进行了强制类型转换**

	强制类型转换时引发程序运行时错误的一个原因，因此在程序出错时，可能就会想到是不是有哪些强制类型转换出了问题。

C++有以下四种强制类型转换运算符：

### static_cast

基本等价于隐式转换

**可以用于低风险的转换**

- 整型和浮点型
- 字符与整型
- 转换运算符
- *** 空指针转换为任何目标类型的指针 ***

**不可以用于风险较高的转换**

- 不同类型的指针之间的转换
- 整型和指针之间的互相转换
- 不同类型的引用之间的转换

```c++
#include <bits/stdc++.h>
using namespace std;

class CInt
{
public:
	// 转换运算符
	operator int() {
		return m_nInt;
	}
	int m_nInt;
};

int main()
{
	int n = 5;
	float f = 10.0f;
	f = n; // 本质进行了隐式转换

	/* 低风险的转换 */
	// 整型与浮点型
	float ff = static_cast<float>(n);

	// 字符与整型
	char ch = 'a';
	n = static_cast<int>(ch);

	// void*指针的转换
	void *p = nullptr;
	int *pn = static_cast<int *>(p);

	// 转换运算符
	CInt obj;
	int k = static_cast<int>(obj);
	int kk = obj;

	/* 高风险的转换: 不允许，均无法编译通过 */
	int t;
	char *p;
	// 整型与指针转化
	p = t;
	char *ptr = static_cast<char *>(t);
	// 不同指针类型之间转换
	int *pp;
	p = static_cast<char *>(pp);

	return 0;
}
```

*** static_cast可用于基类与派生类之间的转换，但没有运行时类型检查 ***

```c++
#include <bits/stdc++.h>
using namespace std;

class A
{
public:
	int a;
};

class B : public A
{
public:
	int b;
};

int main()
{
	A *father = new A;
	B *son = new B;

	// 父类转子类（不安全）
	son = father;					// 隐式转换失败
	son = static_cast<B *>(father); // 转换成功，但是没有提供运行时检查，依然不安全
	// 子类转父类（安全）
	father = son;					// 成功
	father = static_cast<A *>(son); // 成功
	return 0;
}
```

### dynamic_cast

用于具有**虚函数的基类**与**派生类**之间的指针或引用的转换

- 基类必须具备虚函数

	原因：dynamic_cast时**运行时类型检查**，需要运行时类型信息（RTTI），而这个信息是存储与类的**虚函数表**关系紧密，只有一个类定义了虚函数，才有虚函数表，也就是说会在运行时检测被转换的指针的类型（依赖RTTI）

- **运行时检查**，向下转化时，如果是非法的对于指针返回NULL，对于引用抛异常bad_cast

* **非必要时不要使用dynamic_cast，有额外的函数开销**

常见的转换方式：

- 基类指针或引用转换为派生类指针和引用（**必须使用**dynamic_cast）
- 派生类指针或引用转基类指针或引用（更**推荐使用**static_cast）

```c++
#include <bits/stdc++.h>
using namespace std;

class A
{
public:
	// 转换运算符
	virtual void show()
	{
		cout << "我是父类" << endl;
	}
	int a;
};

class B : public A
{
public:
	virtual void show()
	{
		cout << "我是子类" << endl;
	}
	int b;
};

int main()
{
	A *father = new A;
	B *son = new B;

	// 向下转换：父类转子类（不安全）
	// dynamic_cast会在运行时进行检测，如果是不安全的，会转换失败
	son = father;					 // 隐式转换失败
	son = dynamic_cast<B *>(father); // 转换成功，全
	// 向上转换：子类转父类（安全）
	father = son;					 // 成功
	father = dynamic_cast<A *>(son); // 成功
	return 0;
}
```

### const_cast

* 用于删除 const、volatile特性，四个强制类型转换运算符中唯一能够去除const属性的运算符

	*** 常量对象或者基本数据类型不允许转化为非常量对象，只能通过指针或者引用来修改 ***

```c++
#include <bits/stdc++.h>
using namespace std;

int main()
{
	const int n = 5;
	const string s = "abcdef";
	int k = const_cast<int>(n);	// 错误
    
	// const_cast 中的类型必须是指针，引用，this指针
	int *k = const_cast<int *>(&n);
	*k = 10;
	cout << *k << endl;
	return 0;
}
```

*** 常成员函数中去除this指针的const属性 ***

```c++
#include <bits/stdc++.h>
using namespace std;

class CTest
{
public:
	CTest() : m_nTest(2) {}
	// 常成员函数
	void foo(int nTest) const
	{
		// m_nTest = nTest; // 错误
		const_cast<CTest *>(this)->m_nTest = nTest;
	}
	void show()
	{
		cout << m_nTest << endl;
	}

private:
	int m_nTest;
};

int main()
{
	CTest t;
	t.foo(5);
	t.show();
	return 0;
}
```



### reinterpret_cast

* 几乎什么都可以转,不能丢掉 const、volatile特性
* **用于进行各种不同类型的转换**
* **编译期处理，执行的是逐字节复制的操作**
* **类似于显式强转，后果自负**



## RTTI
运行时类型检查，在C++层面主要体现在dynamic_cast和typeid
> * dynamic_cast
    动态类型转换
> * typeid
    typeid 运算符允许在运行时确定对象的类型，获取对象的实际类型

## RAII
RAII全称是“Resource Acquisition is Initialization”，直译过来是“资源获取即初始化”.

* 在构造函数中申请分配资源，在析构函数中释放资源。因为C++的语言机制保证了，当一个对象创建的时候，自动调用构造函数，当对象超出作用域的时候会自动调用析构函数。所以，在RAII的指导下，我们应该使用类来管理资源，将资源和对象的生命周期绑定。
* RAII的核心思想是将资源或者状态与对象的生命周期绑定，通过C++的语言机制，实现资源和状态的安全管理,智能指针是RAII最好的例子

## CPP11新特性
### nullptr常量

* C++中NULL仅仅是`define NULL 0`的一个宏定义，因此，有时候会产生歧义
    * 比如f（char*）和f（int），参数传NULL的话到底该调用哪个？
    * 事实上，在VS下测试这样的函数重载会优先调用f（int），但是f（char *）也是正确的，因此C++引入nullptr来避免这个问题
* nullptr是一个空指针，可以被转换成其他任意指针的类型

### constexptr常量表达式

* 在编译过程中就能得到计算结果的表达式

### auto类型指示符

- 早在C++98标准中就存在了auto关键字，那时的auto用于声明变量为自动变量，自动变量意为拥有自动的生命期，这是多余的，因为就算不使用auto声明，变量依旧拥有自动的生命期：

	```c++
	int a = 10 ;  //拥有自动生命期
	auto int b = 20 ;//拥有自动生命期
	static int c = 30 ;//延长了生命期
	```

- C++98中的auto多余且极少使用，C++11已经删除了这一用法，取而代之的是全新的auto：变量的自动类型推断。

* 让编译器替我们去分析表达式所属的类型，直接推导
* 尤其是STL中map的迭代器这种很长的类型，适合用auto

### decltype

- 关键字decltype将变量的类型声明为表达式指定的类型

- decltype通常用于模板函数内部

	```c++
	template<class T1, class T2>
	void ft(T1 x, T2 y) {
	    ...
	    ?type?xpy = x + y;
	    decltype(x+y) xpy = x + y;
	}
	```

- decltype为了确定类型，编译器必须遍历一个核对表，假如有如下声明：decltype(expression) var；核对表简化版如下：
	- 如果expression是一个没有用括号括起的标识符，则var的类型与该标识符的类型相同，包括const等限定符；
	- 如果expression是一个函数调用，则var的类型与函数的返回类型相同，实际上并不会调用函数，编译器通过查看函数的原型来获悉返回类型，而无需实际调用函数；
	- 如果expression声明为一个左值，则var为指向其类型的引用，前提是expression用括号括起来了；
	- 如果前面都不满足，则var类型与expression类型相同。

### 委托构造

* C++引入了委托构造的概念，这使得构造函数可以在同一个类中调用另一个构造函数，从而达到简化代码的目的

    ```c++
    #include <bits/stdc++.h>
    using namespace std;
    
    class A
    {
    public:
        int value1;
        int value2;
        float value3;
    public:
        A() {
            value1 = 1;
            cout << "A()" << endl;
        }
        A(float f) : A() {
            value3 = f;
            cout << "A(float)" << endl;
        }
        A(int n) : A(3.14f) {
            value2 = n;
            cout << "A(int)" << endl;
        }
    };
    
    int main() {
        A(5);
        return 0;
    }
    ```

### 继承构造

- 在传统C++中，构造函数如果需要继承是需要将参数一一传递的，这将导致效率低下，C++利用关键字using引入了继承构造的概念。派生类能够通过using语句声明要在子类中继承基类的全部构造函数。

	```c++
	#include <bits/stdc++.h>
	using namespace std;
	
	class A
	{
	public:
	    int value1;
	    int value2;
	    float value3;
	public:
	    A() {
	        value1 = 1;
	        cout << "A()" << endl;
	    }
	    A(float d) : A() {
	        value3 = d;
	        cout << "A(float)" << endl;
	    }
	    A(int n) : A(3.14f) {
	        value2 = n;
	        cout << "A(int)" << endl;
	    }
	};
	
	class B : public A
	{
	public:
	    using A::A; 		// 继承构造
	    B(int n) : A(n) {}	// 显式声明A的构造函数
	};
	
	int main() {
	    B b(1);
	    cout << b.value1 << endl;
	    cout << b.value2 << endl;
	    cout << b.value3 << endl;
	
	    return 0;
	}
	```

### override保留字

- override保留字表示当前函数重写了基类的虚函数。 
- 在函数比较多的情况下可以提示读者某个函数重写了基类虚函数,表示这个虚函数是从基类继承,不是派生类自己定义的。
- 强制编译器检查某个函数是否重写基类虚函数,如果没有则报错。

### final关键字

- `禁用继承`：C++11中允许将类标记为final，方法时直接在类名称后面使用关键字final，如此，意味着继承该类会导致编译错误。
- `禁用重写`：C++中还允许将方法标记为fianal，这意味着无法再子类中重写该方法。这时final关键字至于方法参数列表后面

### 范围for语句

* 多与auto配合使用

```C++
string str("somthing");
for(auto i:str) //对于str中的每个字符，i类型为char
    cout << c << endl;

for(auto &i:str) //对于若要改变每个字符的值，需要加引用
    cout << c << endl;
```

### 定义双层vector

* `vector<vector<int>>(m, vector<int>(n, 0))` 创建m行n列的二维数组，全部初始化为0

### 仿函数

* 定义
	* 仿函数(functor)又称之为函数对象（function object），其实就是重载了operator()操作符的struct或class，是一个能行使函数功能的类
	* 它使一个类的使用看上去像一个函数，这个类就有了类似函数的行为，就是一个仿函数类。 
* 优点：仿函数由于是对象，因此可以保存函数状态，有非常大的优势。
* 缺点：需要显式定义一个类。

### [lambda表达式](https://blog.csdn.net/qq_43265890/article/details/83218413)

* 用于实现匿名函数，匿名函数只有函数体，没有函数名

```C++
[capture list] (params list) mutable exception-> return type {function body};  //1
[capture list] (params list) -> return type {function body};  //1 省略mutable，表示const不可修改
[capture list] (params list) {function body};		//2 省略返回类型，按照函数体返回值决定返回类型
[capture list] {function body};		//3 省略参数列表，无参函数
```
* 参数
    * capture list：捕获外部变量列表
    * params list：形参列表
    * mutable指示符：用来说用是否可以修改捕获的变量，可选
    * exception：异常设定
    * return type：返回类型
    * function body：函数体

```C++
//示例
sort(vec.begin(), vec.end(), [](int a, int b)->bool{return a < b})
```
* 参数捕获方式
  
  * 值捕获(传参)
  * 引用捕获（传引用）
  * 按值捕获所有变量（传=符号）
  * 按引用捕获所有变量（传&符号）
  
* 匿名函数一般只有在定义的时候才能使用，为了保证后续还可以使用，可以使用auto类型推导

    ```c++
    auto f = [](int a, int b) -> int {return a + b};
    ```

- 嵌套lambda表达式

	```c++
	auto f = [](int n) {
		return [n](int x) {
			return n+x; 
		};
	};
	int c = f(1)(2);
	```

**为什么要用lambda代替仿函数？**

在使用仿函数的时候不仅需要单独写一个类，还要重载运算符，这样的话就会导致需要写的代码非常多，而且还要起类名。而使用lambda可以简化这些操作，而且lambda还可以捕获所在作用域中的局部变量，以供其内部使用。

### 函数对象包装器function

为函数提供了一种容器（封装），std :: function就是C++中用来代替C函数指针的。function支持封装四种函数，分别为普通函数、匿名函数、类成员函数、仿函数。

```c++
int test(int n) {
    cout << n << endl;
    return n;
}

class Test
{
public:
    int test(int n) {
        cout << n << endl;
        return n;
    }
    int operator()(int n) {
        cout << n << endl;
        return n;
    }
};

int main() {
    // 普通函数
    std::function<int(int)> f1 = test;
    f1(2);
    // 匿名函数
    std::function<int(int)> f2 = [](int n) -> int {
        cout << n << endl;
        return n;
    };
    f2(2);
    // 类的成员函数，注意，需要传递this指针
    std::function<int(Test*, int)> f3 = &Test::test;
    Test t;
    f3(&t, 2);
    // 仿函数
    std::function<int(Test*, int)> f4 = &Test::operator();
    f4(&t, 3);
    return 0;
}
```

### bind绑定

将函数和函数参数绑定在一起，解决参数较多的函数多次调用时代码冗余的问题

```c++
#include <bits/stdc++.h>
using namespace std;

int test(int a, int b, int c) {
    cout << a << b << c << endl;
    return a*100+b*10+c;
}

int main() {

    auto f1 = std::bind(test, 1, 2, 3);
    f1();   // 123

    auto f2 = std::bind(test, std::placeholders::_1, 2, 3);
    f2(4);  // 423

    auto f3 = std::bind(test, std::placeholders::_2, std::placeholders::_1, 3);
    f3(4, 5);  // 543

    return 0;
}
```

bind有点像默认参数，但是默认参数顺序必须一致，而bind可以参数顺序不一致。

### 右值引用

#### 左值和右值

左值和右值的区分标准在于**能否获取地址**。

左值的定义表示的是可以获取地址的表达式（如`变量名和解除引用的指针`），它能出现在赋值语句的左边，对该表达式进行赋值。

左值引用使用==&==符号，可以使用左值引用关联左值。

```c++
int & j = *p; // 合法，*p是解除引用的指针，是左值
int & k = i;  // 合法，i是变量，是左值
int & i = 0;  // 不合法，0是右值
```

右值表示无法获取地址的对象，有`常量值、函数返回值、表达式`等。无法获取地址，但不表示其不可改变。

而右值引用使用符号==&&==，可以使用右值引用关联右值。

```c++
int && r1 = 13;	// 合法，13是常量，是右值
double && r2 = std::sqrt(2.0); // 合法，std::sqrt(2.0)是函数返回值，是右值
```

const左值引用

int & i = 10; 会报错，但是const int & i = 10; 就不会报错，const修饰左值引用可以取得地址，但没法进行赋值。

### 移动语义

转让所有权而非直接复制，从而避免了对原数据的拷贝

提供一个移动构造函数和一个移动赋值运算符

可使用static_cast<>将左值对象强制转换为右值，或者使用std::move()