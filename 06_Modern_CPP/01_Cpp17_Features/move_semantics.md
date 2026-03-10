# 移动语义与右值引用 (Move Semantics)

> **学习目标**：理解左值与右值的区别，掌握移动构造函数和 `std::move` 的使用，能够编写高效的 C++ 代码

---

## 一、为什么需要移动语义？

### 1.1 深拷贝的性能问题

```cpp
#include <vector>
#include <chrono>
#include <iostream>

// ========== C++98: 深拷贝问题 ==========
class MyVector {
public:
    MyVector(size_t size) : size_(size), data_(new int[size]) {
        for (size_t i = 0; i < size; ++i) {
            data_[i] = static_cast<int>(i);
        }
    }
    
    // 拷贝构造函数：深拷贝
    MyVector(const MyVector& other) 
        : size_(other.size_), data_(new int[other.size_]) {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
        std::cout << "Copy constructor (expensive!)" << std::endl;
    }
    
    ~MyVector() {
        delete[] data_;
    }
    
private:
    size_t size_;
    int* data_;
};

// 问题场景
MyVector create_vector() {
    MyVector temp(1000000);  // 分配 1MB
    return temp;  // C++98: 这里会发生深拷贝！
}

void c98_example() {
    MyVector v = create_vector();  // 不必要的深拷贝
}  // 临时对象和 v 都释放内存

// ========== C++11: 移动语义 ==========
class MyVectorMove {
public:
    MyVectorMove(size_t size) : size_(size), data_(new int[size]) {}
    
    // 拷贝构造函数
    MyVectorMove(const MyVectorMove& other) 
        : size_(other.size_), data_(new int[other.size_]) {
        std::cout << "Copy constructor" << std::endl;
    }
    
    // 移动构造函数：窃取资源
    MyVectorMove(MyVectorMove&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.data_ = nullptr;  // 重要：让 other 不再拥有资源
        other.size_ = 0;
        std::cout << "Move constructor (cheap!)" << std::endl;
    }
    
    ~MyVectorMove() {
        delete[] data_;
    }
    
private:
    size_t size_;
    int* data_;
};

MyVectorMove create_vector_move() {
    MyVectorMove temp(1000000);
    return temp;  // C++11: 自动使用移动构造（RVO/NRVO）
}

void c11_example() {
    MyVectorMove v = create_vector_move();  // 移动而非拷贝
}  // 只释放一次内存
```

---

## 二、左值与右值

### 2.1 核心概念

```
┌─────────────────────────────────────────────────────────────┐
│                   左值 (lvalue) vs 右值 (rvalue)             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  左值 (lvalue)                   右值 (rvalue)               │
│  ┌─────────────────┐            ┌─────────────────┐         │
│  │ 有名字的对象     │            │ 临时对象         │         │
│  │ 可以取地址       │            │ 通常不能取地址   │         │
│  │ 可以多次使用     │            │ 使用后即销毁     │         │
│  │ 出现在赋值左边   │            │ 出现在赋值右边   │         │
│  └─────────────────┘            └─────────────────┘         │
│                                                             │
│  示例：                          示例：                      │
│  int x = 5;                    5                           │
│  x = 10;                       x + 1                       │
│  &x  // 有效                   &(x + 1)  // 无效！         │
│                                                             │
│  生命周期：                    生命周期：                   │
│  直到作用域结束                 当前表达式结束               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 左值与右值示例

```cpp
#include <iostream>
#include <string>
#include <vector>

int main() {
    // ===== 左值示例 =====
    int x = 42;
    int& ref = x;        // 左值引用可以绑定到左值
    int* ptr = &x;       // 可以取地址
    
    std::string str = "hello";
    str = "world";       // 可以赋值
    
    std::vector<int> vec = {1, 2, 3};
    vec.push_back(4);    // 可以修改
    
    // ===== 右值示例 =====
    int y = x + 1;       // x + 1 是右值（临时对象）
    
    std::string temp = str + "!";  // str + "!" 是右值
    
    // 右值引用
    int&& rref = 42;     // 右值引用绑定到字面量
    int&& rref2 = x + 1; // 绑定到表达式结果
    
    // ===== 重要规则 =====
    // 左值引用不能绑定到右值
    // int& bad_ref = 42;  // 编译错误！
    
    // const 左值引用可以绑定到右值
    const int& const_ref = 42;  // OK，生命周期延长
    
    // 右值引用不能绑定到左值
    // int&& bad_rref = x;  // 编译错误！
    
    return 0;
}
```

### 2.3 值类别详解（C++11 起）

```cpp
#include <iostream>
#include <type_traits>

// ===== 五种值类别 =====

// 1. lvalue (左值): 有身份，不可移动
int x = 42;
int& get_ref() { return x; }  // 返回左值引用

// 2. xvalue (将亡值): 有身份，可移动
int&& get_rref() { return std::move(x); }  // 返回右值引用

// 3. glvalue = lvalue + xvalue (广义左值)

// 4. rvalue (纯右值): 无身份，可移动
int get_value() { return 42; }  // 返回临时对象

// 5. prvalue = rvalue - xvalue (纯右值)

// ===== 类型推导规则 =====
template<typename T>
void analyze_type(T&& param) {
    // T&& 是万能引用 (forwarding reference)
    
    if constexpr (std::is_lvalue_reference_v<T>) {
        std::cout << "T is lvalue reference" << std::endl;
    }
    if constexpr (std::is_rvalue_reference_v<T>) {
        std::cout << "T is rvalue reference" << std::endl;
    }
}

int main() {
    int a = 42;
    
    analyze_type(a);           // T = int& (左值)
    analyze_type(std::move(a)); // T = int&& (右值)
    analyze_type(42);          // T = int (纯右值)
    
    return 0;
}
```

---

## 三、移动构造函数与移动赋值

### 3.1 移动构造函数

```cpp
#include <iostream>
#include <utility>

class Buffer {
public:
    // 构造函数
    explicit Buffer(size_t size) 
        : size_(size), data_(new int[size]) {
        std::cout << "Constructor: " << size_ << " elements" << std::endl;
    }
    
    // 拷贝构造函数
    Buffer(const Buffer& other) 
        : size_(other.size_), data_(new int[other.size_]) {
        std::cout << "Copy constructor" << std::endl;
        std::copy(other.data_, other.data_ + size_, data_);
    }
    
    // 移动构造函数
    Buffer(Buffer&& other) noexcept  // noexcept 重要！
        : size_(other.size_), data_(other.data_) {
        std::cout << "Move constructor" << std::endl;
        
        // 关键：让 other 处于有效但未定义状态
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    // 拷贝赋值运算符
    Buffer& operator=(const Buffer& other) {
        std::cout << "Copy assignment" << std::endl;
        
        if (this != &other) {
            delete[] data_;  // 释放旧资源
            
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
        }
        
        return *this;
    }
    
    // 移动赋值运算符
    Buffer& operator=(Buffer&& other) noexcept {
        std::cout << "Move assignment" << std::endl;
        
        if (this != &other) {
            delete[] data_;  // 释放旧资源
            
            // 窃取 other 的资源
            data_ = other.data_;
            size_ = other.size_;
            
            // 让 other 处于有效但未定义状态
            other.data_ = nullptr;
            other.size_ = 0;
        }
        
        return *this;
    }
    
    ~Buffer() {
        std::cout << "Destructor" << std::endl;
        delete[] data_;
    }
    
    size_t size() const { return size_; }
    int* data() const { return data_; }
    
private:
    size_t size_;
    int* data_;
};

// 使用示例
Buffer create_buffer() {
    Buffer temp(1000);
    return temp;  // 可能触发移动或 RVO
}

void move_example() {
    Buffer b1(100);
    
    // 拷贝：深拷贝
    Buffer b2 = b1;
    
    // 移动：窃取资源
    Buffer b3 = std::move(b1);
    
    // b1 现在处于有效但未定义状态
    // b1.data() 返回 nullptr
}
```

### 3.2 规则五 (Rule of Five)

```cpp
// 如果你需要定义以下任何一个，通常需要定义全部五个：
// 1. 析构函数
// 2. 拷贝构造函数
// 3. 拷贝赋值运算符
// 4. 移动构造函数
// 5. 移动赋值运算符

class ResourceHolder {
public:
    // 1. 构造函数
    explicit ResourceHolder(size_t size) : data_(new int[size]) {}
    
    // 2. 析构函数
    ~ResourceHolder() {
        delete[] data_;
    }
    
    // 3. 拷贝构造函数
    ResourceHolder(const ResourceHolder& other)
        : data_(new int[100]) {
        // 深拷贝
    }
    
    // 4. 拷贝赋值
    ResourceHolder& operator=(const ResourceHolder& other) {
        if (this != &other) {
            delete[] data_;
            data_ = new int[100];
            // 深拷贝
        }
        return *this;
    }
    
    // 5. 移动构造函数
    ResourceHolder(ResourceHolder&& other) noexcept
        : data_(other.data_) {
        other.data_ = nullptr;
    }
    
    // 6. 移动赋值
    ResourceHolder& operator=(ResourceHolder&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            other.data_ = nullptr;
        }
        return *this;
    }
    
private:
    int* data_;
};

// ===== 现代 C++ 简化方案 =====
// 使用智能指针，让编译器自动生成移动操作

class ModernResourceHolder {
public:
    explicit ModernResourceHolder(size_t size)
        : data_(std::make_unique<int[]>(size)) {}
    
    // 编译器自动生成：
    // - 移动构造函数
    // - 移动赋值运算符
    // - 拷贝操作被删除（unique_ptr 不可拷贝）
    
private:
    std::unique_ptr<int[]> data_;
};
```

---

## 四、std::move 的正确使用

### 4.1 std::move 是什么？

```cpp
#include <utility>
#include <iostream>

// std::move 的本质：将左值转换为右值引用
// 它不移动任何东西！只是类型转换

template<typename T>
typename std::remove_reference<T>::type&& move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

// C++14 简化版
template<typename T>
std::remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}

void move_example() {
    std::string s1 = "hello";
    
    // std::move(s1) 将 s1 转换为右值引用
    // 可以绑定到移动构造函数
    std::string s2 = std::move(s1);
    
    // s1 现在处于有效但未定义状态
    // 可以赋值，但不应假设其值
    s1 = "new value";  // OK
    // 但不应使用 s2 = s1; 因为 s1 的值未定义
}
```

### 4.2 何时使用 std::move

```cpp
#include <vector>
#include <string>
#include <memory>

class MyClass {
public:
    // ✅ 场景 1: 转移所有权
    void set_data(std::unique_ptr<int> ptr) {
        data_ = std::move(ptr);  // 转移所有权
    }
    
    // ✅ 场景 2: 返回值优化失败时
    std::vector<int> get_data() {
        std::vector<int> local = {1, 2, 3};
        return std::move(local);  // 明确使用移动
        // 但通常不需要，编译器会 RVO
    }
    
    // ✅ 场景 3: 交换操作
    void swap(MyClass& other) {
        using std::swap;
        swap(data_, other.data_);
    }
    
    // ⚠️ 场景 4: 不要对返回局部变量的函数使用
    std::string bad_example() {
        std::string local = "hello";
        // return std::move(local);  // 不推荐！阻止 RVO
        return local;  // 推荐：让编译器决定
    }
    
    // ⚠️ 场景 5: 不要对 const 对象使用
    void const_example(const std::string& s) {
        // std::move(s) 会调用拷贝构造而非移动
        // 因为 const 对象不能移动
        std::string copy = s;  // 直接拷贝
    }
    
private:
    std::unique_ptr<int> data_;
};
```

### 4.3 完美转发

```cpp
#include <utility>
#include <memory>

// ===== 万能引用 (Forwarding Reference) =====
template<typename T>
void wrapper(T&& arg) {
    // T&& 在模板中是万能引用
    // 可以绑定到左值和右值
    
    // 完美转发：保持原始值类别
    process(std::forward<T>(arg));
}

// ===== 工厂函数示例 =====
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// 使用示例
class Complex {
public:
    Complex(int a, double b, const std::string& s) {}
};

void factory_example() {
    // 完美转发参数
    auto ptr = make_unique<Complex>(42, 3.14, "hello");
}
```

---

## 五、嵌入式应用示例

### 5.1 移动语义在缓冲区管理中的应用

```cpp
#include <cstdint>
#include <utility>

// ===== 可移动的 DMA 缓冲区 =====
class DmaBuffer {
public:
    explicit DmaBuffer(size_t size)
        : size_(size), data_(new uint8_t[size]) {}
    
    // 移动构造函数
    DmaBuffer(DmaBuffer&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    // 移动赋值
    DmaBuffer& operator=(DmaBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    // 禁止拷贝（大缓冲区不应拷贝）
    DmaBuffer(const DmaBuffer&) = delete;
    DmaBuffer& operator=(const DmaBuffer&) = delete;
    
    ~DmaBuffer() {
        delete[] data_;
    }
    
    uint8_t* data() { return data_; }
    size_t size() const { return size_; }
    
private:
    size_t size_;
    uint8_t* data_;
};

// ===== 缓冲区池 =====
class BufferPool {
public:
    explicit BufferPool(size_t buffer_size, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            available_.push_back(DmaBuffer(buffer_size));
        }
    }
    
    // 获取缓冲区（移动语义）
    DmaBuffer acquire() {
        if (available_.empty()) {
            return DmaBuffer(1024);  // 默认大小
        }
        // 移动返回，无拷贝
        return std::move(available_.back());
    }
    
    // 返回缓冲区
    void release(DmaBuffer buffer) {
        available_.push_back(std::move(buffer));
    }
    
private:
    std::vector<DmaBuffer> available_;
};
```

### 5.2 消息队列中的移动语义

```cpp
#include <queue>
#include <string>
#include <utility>

// ===== 高效消息队列 =====
template<typename T>
class MessageQueue {
public:
    // 发送消息（移动）
    void send(T message) {
        queue_.push(std::move(message));
    }
    
    // 接收消息（移动）
    T receive() {
        T msg = std::move(queue_.front());
        queue_.pop();
        return msg;  // RVO 或移动
    }
    
    // 尝试接收
    bool try_receive(T& out) {
        if (queue_.empty()) {
            return false;
        }
        out = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    bool empty() const { return queue_.empty(); }
    size_t size() const { return queue_.size(); }
    
private:
    std::queue<T> queue_;
};

// 使用示例
struct SensorData {
    uint32_t timestamp;
    float values[8];
    
    // 移动操作
    SensorData(SensorData&&) noexcept = default;
    SensorData& operator=(SensorData&&) noexcept = default;
    
    // 禁止拷贝（大数据）
    SensorData(const SensorData&) = delete;
    SensorData& operator=(const SensorData&) = delete;
};

MessageQueue<SensorData> sensor_queue;

void producer() {
    SensorData data;
    // 填充数据...
    sensor_queue.send(std::move(data));
}

void consumer() {
    SensorData data;
    if (sensor_queue.try_receive(data)) {
        // 处理数据（无拷贝）
    }
}
```

---

## 六、常见陷阱

### 6.1 陷阱与解决方案

```cpp
#include <vector>
#include <string>
#include <memory>

// ⚠️ 陷阱 1: 移动后继续使用对象
void trap1() {
    std::string s1 = "hello";
    std::string s2 = std::move(s1);
    
    // s1 现在处于有效但未定义状态
    // 不要假设 s1 仍然是 "hello"
    // std::cout << s1;  // 未定义的行为！
    
    // ✅ 正确：移动后重新赋值
    s1 = "new value";
}

// ⚠️ 陷阱 2: 对 const 对象使用 std::move
void trap2(const std::string& s) {
    // std::move(s) 不会移动！
    // 因为 const 对象不能修改
    std::string copy = std::move(s);  // 实际调用拷贝构造
}

// ⚠️ 陷阱 3: 返回局部变量时使用 std::move
std::string trap3() {
    std::string local = "hello";
    
    // return std::move(local);  // 不推荐！
    // 这会阻止 RVO (Return Value Optimization)
    
    return local;  // 推荐：编译器会自动优化
}

// ⚠️ 陷阱 4: 忘记 noexcept
class BadMove {
public:
    // 移动构造函数应该 noexcept
    BadMove(BadMove&& other) {  // 没有 noexcept
        // 如果这里抛出异常，可能导致资源泄漏
    }
};

// ✅ 正确做法
class GoodMove {
public:
    GoodMove(GoodMove&& other) noexcept {
        // 移动操作不应抛出异常
    }
};

// ⚠️ 陷阱 5: 移动后未将源对象置为有效状态
class InvalidMove {
public:
    InvalidMove(InvalidMove&& other)
        : data_(other.data_) {
        // 忘记设置 other.data_ = nullptr
        // 导致双重释放！
    }
    
    ~InvalidMove() {
        delete data_;  // 两个对象都释放同一内存！
    }
    
private:
    int* data_;
};

// ✅ 正确做法
class ValidMove {
public:
    ValidMove(ValidMove&& other) noexcept
        : data_(other.data_) {
        other.data_ = nullptr;  // 重要！
    }
    
    ~ValidMove() {
        delete data_;
    }
    
private:
    int* data_;
};
```

---

## 七、实战练习

### 练习 1: 实现可移动的字符串类

```cpp
#include <cstring>
#include <utility>

class MyString {
public:
    // 构造函数
    explicit MyString(const char* str = "") {
        size_ = std::strlen(str);
        data_ = new char[size_ + 1];
        std::strcpy(data_, str);
    }
    
    // 析构函数
    ~MyString() {
        delete[] data_;
    }
    
    // 拷贝构造函数
    MyString(const MyString& other)
        : size_(other.size_), data_(new char[size_ + 1]) {
        std::strcpy(data_, other.data_);
    }
    
    // 拷贝赋值
    MyString& operator=(const MyString& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new char[size_ + 1];
            std::strcpy(data_, other.data_);
        }
        return *this;
    }
    
    // 移动构造函数
    MyString(MyString&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    // 移动赋值
    MyString& operator=(MyString&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    const char* c_str() const { return data_; }
    size_t size() const { return size_; }
    
private:
    size_t size_;
    char* data_;
};
```

### 练习 2: 实现移动感知的容器

```cpp
#include <memory>
#include <algorithm>

template<typename T>
class SimpleVector {
public:
    SimpleVector() : data_(nullptr), size_(0), capacity_(0) {}
    
    ~SimpleVector() {
        clear();
    }
    
    // 移动构造函数
    SimpleVector(SimpleVector&& other) noexcept
        : data_(other.data_),
          size_(other.size_),
          capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    
    // 移动赋值
    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            clear();
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    // 禁止拷贝
    SimpleVector(const SimpleVector&) = delete;
    SimpleVector& operator=(const SimpleVector&) = delete;
    
    void push_back(T value) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        data_[size_++] = std::move(value);
    }
    
    T pop_back() {
        return std::move(data_[--size_]);
    }
    
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
private:
    void reserve(size_t new_capacity) {
        T* new_data = new T[new_capacity];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }
    
    void clear() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        delete[] data_;
    }
    
    T* data_;
    size_t size_;
    size_t capacity_;
};
```

---

## 八、编译与测试

```bash
# 编译命令
g++ -std=c++17 -Wall -Wextra -Wpedantic -o move_test move_semantics.cpp

# 运行测试
./move_test

# 性能对比
time ./move_test

# 内存检查
valgrind --leak-check=full ./move_test
```

---

## 九、关键要点总结

| 要点 | 说明 |
|-----|------|
| ✅ 左值有名字，右值是临时对象 | 理解值类别是基础 |
| ✅ 移动构造函数窃取资源 | 避免深拷贝，提高性能 |
| ✅ 移动操作标记 `noexcept` | 让 STL 容器使用移动 |
| ✅ `std::move` 只是类型转换 | 不实际移动任何东西 |
| ✅ 移动后对象处于有效但未定义状态 | 不应继续使用 |
| ⚠️ 不要对 const 对象使用 `std::move` | 会退化为拷贝 |
| ⚠️ 返回局部变量不需要 `std::move` | 让编译器 RVO |

---

## 十、下一步

完成本章节后，继续学习：
- [Lambda 表达式](./lambda.md)
- [实战练习](../practice/README.md)
