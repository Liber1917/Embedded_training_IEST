# CS106L 核心概念总结

> 基于斯坦福 CS106L 课程的核心概念整理，针对嵌入式系统进行了适配

---

## 一、auto 类型推导

### 核心规则

```cpp
// 1. 基本推导
auto x = 42;           // int
auto y = 3.14;         // double
auto z = "hello";      // const char*

// 2. 与引用组合
int val = 42;
auto& ref = val;       // int&
const auto& cref = val;// const int&

// 3. 与指针组合
int* ptr = &val;
auto p = ptr;          // int*
auto* p2 = &val;       // int*

// 4. 初始化列表
auto list = {1, 2, 3}; // std::initializer_list<int>
```

### 最佳实践

| 场景 | 推荐用法 | 理由 |
|-----|---------|-----|
| 迭代器 | `for (auto it = vec.begin(); ...)` | 类型冗长 |
| 范围 for | `for (const auto& x : vec)` | 避免拷贝 |
| Lambda | `auto lambda = [](int x) {...}` | 类型复杂 |
| 工厂函数 | `auto ptr = std::make_unique<T>()` | 类型明显 |

### 常见陷阱

```cpp
// ⚠️ vector<bool> 返回 proxy
std::vector<bool> flags = {true, false};
auto flag = flags[0];  // bool, 不是引用！

// ⚠️ 窄化转换
auto x = 3.14;
int i = x;  // 隐式转换，可能丢失精度

// ✅ 显式转换
auto i = static_cast<int>(x);
```

---

## 二、统一初始化 (Uniform Initialization)

### 语法

```cpp
// C++98 多种初始化方式
int x1 = 5;
int x2(5);
int x3 = int(5);

// C++11 统一使用 {}
int x1{5};
int x2{5};
std::vector<int> v{1, 2, 3};

// 防止窄化转换
int bad{3.14};  // 编译错误！
```

### 初始化列表

```cpp
#include <initializer_list>

void print_list(std::initializer_list<int> list) {
    for (auto val : list) {
        std::cout << val << std::endl;
    }
}

print_list({1, 2, 3, 4, 5});
```

---

## 三、范围 for 循环 (Range-based for)

### 基本语法

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};

// 只读
for (const auto& val : vec) {
    std::cout << val << std::endl;
}

// 修改
for (auto& val : vec) {
    val *= 2;
}

// 需要索引时使用传统 for
for (size_t i = 0; i < vec.size(); ++i) {
    std::cout << "vec[" << i << "] = " << vec[i] << std::endl;
}
```

### 嵌入式应用

```cpp
// LED 数组遍历
std::array<bool, 8> leds{};
for (auto& led : leds) {
    led = false;  // 关闭所有 LED
}

// 寄存器配置序列
constexpr std::array<RegConfig, 3> configs{{
    {0x01, 0x10}, {0x02, 0x20}, {0x03, 0x30}
}};
for (const auto& cfg : configs) {
    write_register(cfg.addr, cfg.value);
}
```

---

## 四、智能指针

### 选择指南

```
                    需要指针吗？
                        │
           ┌────────────┴────────────┐
           │                         │
          否                        是
           │                         │
    使用值类型/optional              │
                           需要共享所有权吗？
                                │
                    ┌───────────┴───────────┐
                    │                       │
                   否                      是
                    │                       │
            使用 unique_ptr          可能循环引用吗？
                    │                       │
                    │             ┌─────────┴─────────┐
                    │             │                   │
                    │            是                  否
                    │             │                   │
                    │     shared_ptr + weak_ptr   shared_ptr
                    │
            需要自定义删除器吗？
                    │
            ┌───────┴───────┐
            │               │
           是              否
            │               │
    unique_ptr +       unique_ptr
    自定义删除器
```

### 使用示例

```cpp
#include <memory>

// unique_ptr - 独占所有权
auto ptr1 = std::make_unique<int>(42);
auto arr = std::make_unique<int[]>(100);

// shared_ptr - 共享所有权
auto ptr2 = std::make_shared<int>(42);
auto ptr3 = ptr2;  // 引用计数 +1

// weak_ptr - 观察
std::weak_ptr<int> weak = ptr2;
if (auto locked = weak.lock()) {
    std::cout << *locked << std::endl;
}

// 自定义删除器
auto deleter = [](FILE* f) { std::fclose(f); };
std::unique_ptr<FILE, decltype(deleter)> file(
    std::fopen("test.txt", "r"), deleter
);
```

---

## 五、移动语义

### 核心概念

```cpp
// 左值：有名字，可取地址
int x = 42;
int& ref = x;

// 右值：临时对象
int y = x + 1;  // x + 1 是右值

// 右值引用
int&& rref = 42;
int&& rref2 = std::move(x);
```

### 移动构造函数

```cpp
class Buffer {
public:
    // 移动构造函数
    Buffer(Buffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;  // 重要！
        other.size_ = 0;
    }
    
    // 移动赋值
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
private:
    int* data_;
    size_t size_;
};
```

### std::move 使用

```cpp
// 何时使用
void transfer(std::unique_ptr<int> ptr) {
    storage_ = std::move(ptr);  // 转移所有权
}

// 何时不使用
std::vector<int> create() {
    std::vector<int> local{1, 2, 3};
    return local;  // 不要 std::move，让编译器 RVO
}
```

---

## 六、Lambda 表达式

### 捕获列表

| 语法 | 含义 | 使用场景 |
|-----|------|---------|
| `[]` | 空捕获 | 不依赖外部变量 |
| `[=]` | 值捕获所有 | 不修改外部变量 |
| `[&]` | 引用捕获所有 | 需要修改外部变量 |
| `[=, &x]` | 默认值，x 引用 | 大部分值捕获 |
| `[&, x]` | 默认引用，x 值 | 大部分引用捕获 |
| `[x = expr]` | 初始化捕获 | C++14，移动捕获 |

### 嵌入式回调

```cpp
// 定时器回调
Timer::start(1000, [&count]() {
    count++;
});

// 中断处理
InterruptManager::register_irq(0, []() {
    // 处理中断
});

// 事件系统
events.subscribe([](const Event& e) {
    handle_event(e);
});
```

---

## 七、标准库容器

### 容器选择

```
┌─────────────────────────────────────────────────────────────┐
│                    容器选择决策树                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  需要固定大小吗？                                            │
│      │                                                      │
│      ├─ 是 ──→ std::array                                   │
│      │           • 编译期确定大小                           │
│      │           • 无动态分配                               │
│      │           • 嵌入式首选                               │
│      │                                                      │
│      └─ 否                                                  │
│           │                                                 │
│           ▼                                                 │
│  需要快速随机访问吗？                                        │
│      │                                                      │
│      ├─ 是 ──→ std::vector                                  │
│      │           • O(1) 随机访问                            │
│      │           • 末尾插入/删除 O(1)                       │
│      │           • 内存连续                                 │
│      │                                                      │
│      └─ 否                                                  │
│           │                                                 │
│           ▼                                                 │
│  需要频繁中间插入/删除吗？                                   │
│      │                                                      │
│      ├─ 是 ──→ std::list / std::forward_list                │
│      │           • O(1) 插入/删除                           │
│      │           • 内存不连续                               │
│      │                                                      │
│      └─ 否                                                  │
│           │                                                 │
│           ▼                                                 │
│  需要键值映射吗？                                            │
│      │                                                      │
│      ├─ 是 ──→ std::map / std::unordered_map                │
│      │           • map: 有序，O(log n)                      │
│      │           • unordered_map: 无序，O(1)                │
│      │                                                      │
│      └─ 否 ──→ std::set / std::unordered_set                │
│                  • 唯一元素集合                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 嵌入式推荐

```cpp
#include <array>
#include <vector>

// 首选 std::array（编译期大小）
std::array<uint8_t, 256> buffer;
std::array<GPIO_Config, 8> gpio_configs;

// 动态大小用 std::vector
std::vector<SensorData> sensor_readings;

// 避免（除非必要）
// std::list - 内存碎片
// std::map - 动态分配多
```

---

## 八、标准库算法

### 常用算法

```cpp
#include <algorithm>

std::vector<int> data = {5, 2, 8, 1, 9};

// 排序
std::sort(data.begin(), data.end());
std::sort(data.begin(), data.end(), std::greater<int>());

// 查找
auto it = std::find(data.begin(), data.end(), 8);
auto even_it = std::find_if(data.begin(), data.end(),
    [](int x) { return x % 2 == 0; });

// 变换
std::transform(data.begin(), data.end(), data.begin(),
    [](int x) { return x * 2; });

// 计数
int count = std::count_if(data.begin(), data.end(),
    [](int x) { return x > 5; });

// 复制
std::copy(data.begin(), data.end(), output.begin());

// 填充
std::fill(data.begin(), data.end(), 0);
```

---

## 九、最佳实践总结

### DO ✅

```cpp
// 使用 auto 简化类型
auto ptr = std::make_unique<int>(42);

// 使用范围 for
for (const auto& item : container) { ... }

// 使用智能指针
auto resource = std::make_unique<Resource>();

// 使用 Lambda 作为回调
timer.start([]() { handle_tick(); });

// 使用 std::array 替代 C 数组
std::array<int, 10> arr;

// 使用 nullptr 替代 NULL
int* ptr = nullptr;

// 使用 override 关键字
void process() override;

// 使用 enum class
enum class State { Idle, Running, Error };
```

### DON'T ❌

```cpp
// 避免裸指针 new/delete
int* ptr = new int(42);  // ❌
delete ptr;

// 避免 C 风格数组
int arr[10];  // ❌

// 避免宏定义常量
#define MAX_SIZE 100  // ❌
constexpr int MAX_SIZE = 100;  // ✅

// 避免 C 风格转换
int x = (int)3.14;  // ❌
int x = static_cast<int>(3.14);  // ✅

// 避免使用 NULL
int* ptr = NULL;  // ❌
int* ptr = nullptr;  // ✅
```

---

## 十、参考资源

| 资源 | 链接 |
|-----|------|
| CS106L 课程 | https://web.stanford.edu/class/cs106l/ |
| cppreference | https://en.cppreference.com/ |
| LearnCpp | https://www.learncpp.com/ |
| Effective Modern C++ | Scott Meyers |

---

*下一步：[d2learn.org 学习指南](../03_d2learn_Guide/README.md)*
