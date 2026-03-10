# 智能指针 (Smart Pointers)

> **学习目标**：掌握 `unique_ptr`、`shared_ptr`、`weak_ptr` 的使用，理解所有权语义，能够用智能指针替代裸指针

---

## 一、为什么需要智能指针？

### 1.1 C 风格指针的问题

```cpp
// ========== C 风格内存管理的问题 ==========

void problematic_function() {
    int* data = new int[100];
    
    // 问题 1: 忘记 delete
    // return;  // 内存泄漏！
    
    // 问题 2: 异常安全
    if (some_condition()) {
        return;  // 如果上面有 new，这里会泄漏
    }
    
    delete[] data;
}

// 问题 3: 所有权不清晰
void process_data(int* data) {
    // data 的所有权是谁？
    // 这个函数应该 delete 它吗？
    process(data);
    // delete data;  // 不确定！
}

// 问题 4: 悬空指针
int* create_data() {
    int local = 42;
    return &local;  // 返回局部变量地址！
}
```

### 1.2 智能指针的解决方案

```cpp
#include <memory>

void safe_function() {
    // 自动释放，无需手动 delete
    auto data = std::make_unique<int[]>(100);
    
    // 即使提前返回也安全
    if (some_condition()) {
        return;  // data 自动释放
    }
    
    // 离开作用域自动释放
}  // data 在这里自动释放
```

---

## 二、std::unique_ptr - 独占所有权

### 2.1 基本概念

`unique_ptr` 表示**独占所有权**：同一时间只有一个指针拥有该对象。

```cpp
#include <iostream>
#include <memory>

int main() {
    // ===== 创建 unique_ptr =====
    
    // 方法 1: 使用 make_unique (推荐，C++14)
    auto ptr1 = std::make_unique<int>(42);
    
    // 方法 2: 直接构造
    std::unique_ptr<int> ptr2(new int(100));
    
    // ===== 访问对象 =====
    std::cout << *ptr1 << std::endl;  // 42
    std::cout << ptr1->operator*() << std::endl;  // 42
    
    // ===== 所有权转移 =====
    // unique_ptr 不能拷贝
    // auto ptr3 = ptr1;  // 编译错误！
    
    // 但可以移动
    auto ptr3 = std::move(ptr1);
    
    // ptr1 现在为空
    if (ptr1 == nullptr) {
        std::cout << "ptr1 is null" << std::endl;
    }
    
    // ptr3 现在拥有对象
    std::cout << *ptr3 << std::endl;  // 42
    
    // ===== 释放所有权 =====
    int* raw = ptr3.release();  // 释放所有权，返回裸指针
    // ptr3 现在为空，raw 需要手动 delete
    delete raw;
    
    // ===== 重置 =====
    ptr3.reset(new int(200));  // 释放旧对象，接管新对象
    ptr3.reset();  // 释放对象，ptr3 变为空
    
    return 0;
}
```

### 2.2 unique_ptr 与数组

```cpp
#include <memory>

// ===== 管理数组 =====
void array_example() {
    // C++14 起支持数组特化
    auto arr = std::make_unique<int[]>(10);
    
    // 访问元素
    for (int i = 0; i < 10; ++i) {
        arr[i] = i * 10;
    }
    
    // 使用 get() 获取裸指针
    int* raw = arr.get();
    
    // 自动释放，无需 delete[]
}  // arr 离开作用域，自动 delete[]

// ===== 与 C API 交互 =====
#include <cstdlib>

void c_api_example() {
    // 分配 C 风格数组
    auto buffer = std::make_unique<char[]>(1024);
    
    // 传递给 C 函数
    snprintf(buffer.get(), 1024, "Hello, %s!", "World");
    
    // 使用 buffer.get()
    printf("%s\n", buffer.get());
    
    // 自动释放
}
```

### 2.3 自定义删除器

```cpp
#include <memory>
#include <cstdio>

// ===== 自定义删除器示例：FILE* =====
struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) {
            std::fclose(f);
            std::cout << "File closed" << std::endl;
        }
    }
};

void file_example() {
    // 使用自定义删除器管理 FILE*
    std::unique_ptr<FILE, FileDeleter> file(
        std::fopen("test.txt", "w"),
        FileDeleter{}
    );
    
    if (file) {
        std::fputs("Hello, World!", file.get());
    }
    // 自动调用 fclose
}

// ===== 嵌入式示例：GPIO 资源管理 =====
struct GpioDeleter {
    void operator()(volatile uint32_t* ptr) const {
        // 释放 GPIO 资源（如果有）
        // 例如：禁用时钟、释放锁等
        if (ptr) {
            // *ptr = 0;  // 重置寄存器
        }
    }
};

void gpio_example() {
    constexpr auto GPIO_BASE = 0x48000000UL;
    
    std::unique_ptr<volatile uint32_t, GpioDeleter> gpio(
        reinterpret_cast<volatile uint32_t*>(GPIO_BASE),
        GpioDeleter{}
    );
    
    // 使用 GPIO
    // *gpio = 0x01;
    
    // 离开作用域自动清理
}
```

### 2.4 嵌入式应用：DMA 缓冲区管理

```cpp
#include <memory>
#include <cstdint>

// 假设的 DMA 缓冲区
struct DmaBuffer {
    static constexpr size_t SIZE = 4096;
    
    // 自定义删除器：确保缓存一致性
    struct Deleter {
        void operator()(uint8_t* ptr) const {
            if (ptr) {
                // 清理缓存（嵌入式特定）
                // __DMB();
                // __DSB();
                delete[] ptr;
            }
        }
    };
    
    using Ptr = std::unique_ptr<uint8_t[], Deleter>;
    
    static Ptr allocate() {
        // 分配对齐的缓冲区
        uint8_t* buffer = new uint8_t[SIZE];
        return Ptr(buffer);
    }
};

void dma_example() {
    auto buffer = DmaBuffer::allocate();
    
    // 填充数据
    for (size_t i = 0; i < DmaBuffer::SIZE; ++i) {
        buffer[i] = static_cast<uint8_t>(i % 256);
    }
    
    // 启动 DMA 传输
    // dma_start(buffer.get(), DmaBuffer::SIZE);
    
    // 等待完成
    // dma_wait();
    
    // 自动释放
}
```

---

## 三、std::shared_ptr - 共享所有权

### 3.1 基本概念

`shared_ptr` 使用**引用计数**实现共享所有权。

```cpp
#include <iostream>
#include <memory>

int main() {
    // ===== 创建 shared_ptr =====
    auto ptr1 = std::make_shared<int>(42);
    
    // ===== 引用计数 =====
    std::cout << "use_count: " << ptr1.use_count() << std::endl;  // 1
    
    // ===== 共享所有权 =====
    auto ptr2 = ptr1;  // 拷贝，引用计数 +1
    std::cout << "use_count: " << ptr1.use_count() << std::endl;  // 2
    
    // ptr1 和 ptr2 指向同一对象
    std::cout << *ptr1 << std::endl;  // 42
    std::cout << *ptr2 << std::endl;  // 42
    
    // ===== 释放 =====
    ptr1.reset();  // 引用计数 -1
    std::cout << "use_count: " << ptr2.use_count() << std::endl;  // 1
    
    // ptr2 仍然可以访问
    std::cout << *ptr2 << std::endl;  // 42
    
    return 0;
}  // ptr2 离开作用域，引用计数变为 0，对象被释放
```

### 3.2 make_shared 的优势

```cpp
#include <memory>

void make_shared_example() {
    // ===== 推荐：使用 make_shared =====
    auto ptr1 = std::make_shared<int>(42);
    
    // 优势 1: 一次内存分配（对象 + 控制块）
    // 优势 2: 异常安全
    // 优势 3: 代码简洁
    
    // ===== 不推荐：直接构造 =====
    std::shared_ptr<int> ptr2(new int(42));
    
    // 问题：两次内存分配
    // 问题：异常不安全
    // process(shared_ptr<int>(new int(42)), other_function());
    // 如果 other_function 抛出，new 的对象会泄漏
}
```

### 3.3 循环引用问题

```cpp
#include <memory>

// ===== 循环引用示例 =====
struct B;  // 前向声明

struct A {
    std::shared_ptr<B> b_ptr;
    ~A() { std::cout << "A destroyed" << std::endl; }
};

struct B {
    std::shared_ptr<A> a_ptr;
    ~B() { std::cout << "B destroyed" << std::endl; }
};

void circular_reference() {
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();
    
    a->b_ptr = b;  // a 持有 b
    b->a_ptr = a;  // b 持有 a - 循环引用！
    
    // 即使 a 和 b 离开作用域，引用计数也不会变为 0
    // 内存泄漏！
}

// ===== 解决方案：使用 weak_ptr =====
// 见下一节
```

---

## 四、std::weak_ptr - 打破循环引用

### 4.1 weak_ptr 基本概念

`weak_ptr` 不增加引用计数，用于观察 `shared_ptr` 管理的对象。

```cpp
#include <iostream>
#include <memory>

int main() {
    auto shared = std::make_shared<int>(42);
    
    // 创建 weak_ptr
    std::weak_ptr<int> weak = shared;
    
    std::cout << "shared use_count: " << shared.use_count() << std::endl;  // 1
    // weak_ptr 不计入 use_count
    
    // ===== 访问对象 =====
    // 方法 1: 锁住（推荐）
    if (auto locked = weak.lock()) {
        std::cout << *locked << std::endl;  // 42
    }
    
    // 方法 2: 检查是否过期
    if (!weak.expired()) {
        std::cout << "Object still exists" << std::endl;
    }
    
    // ===== 对象释放后 =====
    shared.reset();  // 释放对象
    
    std::cout << "weak expired: " << weak.expired() << std::endl;  // true
    
    auto locked = weak.lock();
    if (!locked) {
        std::cout << "Object no longer exists" << std::endl;
    }
    
    return 0;
}
```

### 4.2 解决循环引用

```cpp
#include <memory>
#include <iostream>

struct B;  // 前向声明

struct A {
    std::shared_ptr<B> b_ptr;
    ~A() { std::cout << "A destroyed" << std::endl; }
};

struct B {
    std::weak_ptr<A> a_ptr;  // 使用 weak_ptr 打破循环
    ~B() { std::cout << "B destroyed" << std::endl; }
    
    void access_a() {
        if (auto a = a_ptr.lock()) {
            std::cout << "Accessing A through weak_ptr" << std::endl;
        }
    }
};

void fixed_circular_reference() {
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();
    
    a->b_ptr = b;  // a 持有 b (shared)
    b->a_ptr = a;  // b 观察 a (weak) - 不增加引用计数
    
    // 现在可以正确释放
}  // a 和 b 都会被正确释放
```

### 4.3 嵌入式应用：观察者模式

```cpp
#include <memory>
#include <vector>

// ===== 传感器数据观察者模式 =====
class SensorData {
public:
    int value;
};

class Display {
public:
    std::weak_ptr<SensorData> data;
    
    void update() {
        if (auto sensor_data = data.lock()) {
            // 更新显示
            std::cout << "Display: " << sensor_data->value << std::endl;
        } else {
            std::cout << "Sensor data no longer available" << std::endl;
        }
    }
};

class Sensor {
public:
    std::shared_ptr<SensorData> data;
    std::vector<std::weak_ptr<SensorData>> observers;
    
    void add_observer(std::weak_ptr<SensorData> obs) {
        observers.push_back(obs);
    }
    
    void set_value(int val) {
        data->value = val;
        notify();
    }
    
    void notify() {
        for (auto& obs : observers) {
            // 通知观察者
        }
    }
};
```

---

## 五、智能指针选择指南

```
┌─────────────────────────────────────────────────────────────┐
│                    智能指针选择决策树                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  需要指针吗？                                                │
│      │                                                      │
│      ├─ 否 ──→ 使用值类型或 std::optional                   │
│      │                                                      │
│      └─ 是                                                  │
│           │                                                 │
│           ▼                                                 │
│  需要共享所有权吗？                                          │
│      │                                                      │
│      ├─ 否 ──→ 使用 std::unique_ptr                         │
│      │           • 独占所有权                               │
│      │           • 性能最优                                 │
│      │           • 可添加自定义删除器                        │
│      │                                                      │
│      └─ 是                                                  │
│           │                                                 │
│           ▼                                                 │
│  可能形成循环引用吗？                                        │
│      │                                                      │
│      ├─ 是 ──→ 使用 std::shared_ptr + std::weak_ptr         │
│      │           • 一方用 shared_ptr                        │
│      │           • 另一方用 weak_ptr                        │
│      │                                                      │
│      └─ 否 ──→ 使用 std::shared_ptr                         │
│                  • 共享所有权                               │
│                  • 引用计数管理                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 选择建议表

| 场景 | 推荐 | 理由 |
|-----|------|-----|
| 独占资源 | `unique_ptr` | 性能最优，语义清晰 |
| 工厂函数返回 | `unique_ptr` | 明确转移所有权 |
| 容器存储 | `unique_ptr` | 避免拷贝开销 |
| 共享数据 | `shared_ptr` | 多所有者场景 |
| 观察者/缓存 | `weak_ptr` | 不延长生命周期 |
| C API 包装 | `unique_ptr` + 自定义删除器 | 自动资源管理 |
| 嵌入式资源 | `unique_ptr` + 自定义删除器 | 确保资源释放 |

---

## 六、常见陷阱与最佳实践

### 6.1 常见陷阱

```cpp
#include <memory>

// ⚠️ 陷阱 1: 混用裸指针和智能指针
void trap1() {
    int* raw = new int(42);
    std::unique_ptr<int> ptr(raw);
    
    // delete raw;  // 错误！会导致双重释放
    // 让 unique_ptr 管理
}

// ⚠️ 陷阱 2: 从裸指针创建 shared_ptr
void trap2() {
    int* raw = new int(42);
    std::shared_ptr<int> ptr1(raw);
    std::shared_ptr<int> ptr2(raw);  // 错误！两个独立的控制块
    
    // 正确做法
    auto ptr3 = std::make_shared<int>(42);
    auto ptr4 = ptr3;  // 共享同一控制块
}

// ⚠️ 陷阱 3: this 指针作为 shared_ptr
struct Trap3 : public std::enable_shared_from_this<Trap3> {
    void process() {
        // 错误：不能直接从 this 创建 shared_ptr
        // std::shared_ptr<Trap3> ptr(this);
        
        // 正确：使用 shared_from_this()
        std::shared_ptr<Trap3> ptr = shared_from_this();
    }
};

// ⚠️ 陷阱 4: 在构造函数中捕获 this
struct Trap4 {
    Trap4() {
        // 危险：this 可能还未完全构造
        // timer_.async_wait([this](auto...) { ... });
    }
};
```

### 6.2 最佳实践

```cpp
// ✅ 实践 1: 优先使用 make_unique/make_shared
auto ptr1 = std::make_unique<int>(42);
auto ptr2 = std::make_shared<int>(42);

// ✅ 实践 2: 函数参数使用裸指针或引用
void process_raw(int* ptr);      // 不转移所有权
void process_ref(int& ref);      // 不转移所有权
void process_unique(std::unique_ptr<int> ptr);  // 转移所有权

// ✅ 实践 3: 返回值使用智能指针
std::unique_ptr<int> create_data() {
    return std::make_unique<int>(42);
}

// ✅ 实践 4: 类成员使用智能指针
class MyClass {
    std::unique_ptr<Resource> resource_;  // 独占
    std::weak_ptr<Observer> observer_;    // 观察
};

// ✅ 实践 5: 容器存储智能指针
std::vector<std::unique_ptr<Item>> items;
items.push_back(std::make_unique<Item>());
```

---

## 七、实战练习

### 练习 1: 实现 RAII 文件句柄

```cpp
#include <memory>
#include <cstdio>

// 任务：使用 unique_ptr 和自定义删除器实现文件句柄
// 答案：
struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) {
            std::fclose(f);
        }
    }
};

using FilePtr = std::unique_ptr<FILE, FileDeleter>;

FilePtr open_file(const char* path, const char* mode) {
    return FilePtr(std::fopen(path, mode));
}

void use_file() {
    auto file = open_file("test.txt", "w");
    if (file) {
        std::fputs("Hello!", file.get());
    }
    // 自动关闭
}
```

### 练习 2: 实现对象池

```cpp
#include <memory>
#include <vector>
#include <stack>

// 任务：使用 shared_ptr 实现简单的对象池
template<typename T>
class ObjectPool {
public:
    using Ptr = std::shared_ptr<T>;
    
    ObjectPool(size_t size) {
        for (size_t i = 0; i < size; ++i) {
            available_.push(std::make_shared<T>());
        }
    }
    
    Ptr acquire() {
        if (available_.empty()) {
            return std::make_shared<T>();
        }
        auto obj = available_.top();
        available_.pop();
        
        // 添加自定义删除器，返回时放回池中
        return Ptr(obj.get(), [this, obj](T*) mutable {
            available_.push(obj);
        });
    }
    
private:
    std::stack<Ptr> available_;
};
```

### 练习 3: GPIO 资源管理

```cpp
#include <memory>
#include <cstdint>

// 任务：使用 unique_ptr 管理 GPIO 资源
struct GpioConfig {
    volatile uint32_t* moder;
    volatile uint32_t* otyper;
    volatile uint32_t* odr;
};

struct GpioDeleter {
    void operator()(GpioConfig* config) const {
        if (config) {
            // 重置 GPIO 配置
            if (config->moder) *config->moder = 0;
            if (config->otyper) *config->otyper = 0;
            if (config->odr) *config->odr = 0;
            delete config;
        }
    }
};

using GpioPtr = std::unique_ptr<GpioConfig, GpioDeleter>;

GpioPtr gpio_init() {
    auto config = new GpioConfig{
        reinterpret_cast<volatile uint32_t*>(0x48000000),
        reinterpret_cast<volatile uint32_t*>(0x48000004),
        reinterpret_cast<volatile uint32_t*>(0x48000008)
    };
    return GpioPtr(config);
}
```

---

## 八、编译与测试

```bash
# 编译命令
g++ -std=c++17 -Wall -Wextra -Wpedantic -o smart_ptr_test smart_pointers.cpp

# 内存检查
valgrind --leak-check=full ./smart_ptr_test

# 或使用 AddressSanitizer
g++ -std=c++17 -fsanitize=address -o smart_ptr_test smart_pointers.cpp
./smart_ptr_test
```

---

## 九、关键要点总结

| 要点 | 说明 |
|-----|------|
| ✅ 优先使用 `unique_ptr` | 独占所有权，性能最优 |
| ✅ 使用 `make_unique/make_shared` | 异常安全，代码简洁 |
| ✅ `weak_ptr` 打破循环引用 | 观察者模式必备 |
| ⚠️ 不要混用裸指针和智能指针 | 避免双重释放 |
| ⚠️ 不要从多个裸指针创建 `shared_ptr` | 会导致独立控制块 |

---

## 十、下一步

完成本章节后，继续学习：
- [RAII 惯用法](./raii.md)
- [移动语义](./move_semantics.md)
