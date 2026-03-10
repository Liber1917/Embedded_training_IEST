# auto 类型推导与范围 for 循环

> **学习目标**：掌握 `auto` 类型推导规则，理解范围 for 循环的使用，能够编写更简洁的 C++ 代码

---

## 一、auto 类型推导

### 1.1 什么是 auto？

`auto` 让编译器自动推导变量类型，减少冗余的类型声明。

#### C++98 vs C++17 对比

```cpp
// ========== C++98 风格 ==========
std::vector<std::string>::iterator it = names.begin();
for (std::vector<std::string>::iterator it = names.begin(); 
     it != names.end(); ++it) {
    std::cout << *it << std::endl;
}

// ========== C++17 风格 ==========
auto it = names.begin();
for (auto it = names.begin(); it != names.end(); ++it) {
    std::cout << *it << std::endl;
}
```

### 1.2 auto 基本规则

```cpp
#include <iostream>
#include <vector>
#include <string>

int main() {
    // ===== 基础类型推导 =====
    auto x = 42;           // x 是 int
    auto y = 3.14;         // y 是 double
    auto z = "hello";      // z 是 const char*
    auto s = std::string{"hi"};  // s 是 std::string
    
    // ===== 容器迭代器 =====
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // C++98 风格
    std::vector<int>::iterator it1 = numbers.begin();
    
    // C++11/17 风格
    auto it2 = numbers.begin();  // std::vector<int>::iterator
    
    // ===== 复杂类型简化 =====
    std::vector<std::pair<std::string, int>> data = {
        {"Alice", 25}, {"Bob", 30}
    };
    
    // C++98: 类型声明冗长
    std::vector<std::pair<std::string, int>>::iterator it3 = data.begin();
    
    // C++17: 简洁清晰
    auto it4 = data.begin();
    
    return 0;
}
```

### 1.3 auto 与 const、引用组合

```cpp
#include <iostream>

int main() {
    int value = 42;
    const int const_value = 100;
    
    // ===== auto 推导规则 =====
    auto a = value;           // a 是 int (值拷贝)
    auto b = const_value;     // b 是 int (const 被丢弃)
    
    // ===== 保留 const =====
    auto c = &const_value;    // c 是 const int* (指针本身的 const 保留)
    
    // ===== 显式指定 const/引用 =====
    const auto d = value;     // d 是 const int
    auto& e = value;          // e 是 int&
    const auto& f = const_value;  // f 是 const int&
    
    // ===== 实际应用场景 =====
    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    
    // 只读遍历 - 使用 const auto&
    for (const auto& name : names) {
        std::cout << name << std::endl;  // 不能修改 name
    }
    
    // 需要修改 - 使用 auto&
    for (auto& name : names) {
        name += " Jr.";  // 可以修改
    }
    
    return 0;
}
```

### 1.4 嵌入式系统中的 auto 应用

```cpp
// ===== 寄存器定义示例 =====
#include <cstdint>

// 假设的寄存器结构
struct GPIO_Reg {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
};

// 使用 auto 简化寄存器访问
void gpio_init() {
    constexpr auto GPIO_BASE = 0x48000000UL;
    auto* gpio = reinterpret_cast<GPIO_Reg*>(GPIO_BASE);
    
    // 配置为输出模式
    gpio->MODER = 0x00000001;
    
    // 使用 auto 读取输入
    auto input_value = gpio->IDR;
    
    // 使用 auto 写入输出
    gpio->ODR = input_value;
}

// ===== 回调函数注册 =====
using Callback = void(*)(int);

void register_callback(auto callback) {  // C++20 起
    // ...
}

// C++17 替代方案
template<typename Func>
void register_callback_cpp17(Func callback) {
    // ...
}
```

### 1.5 常见陷阱

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<bool> flags = {true, false, true};
    
    // ⚠️ 陷阱 1: vector<bool> 的特殊性
    // vector<bool> 是特化版本，operator[] 返回 proxy 对象
    auto flag = flags[0];  // flag 是 bool，不是引用！
    
    // ⚠️ 陷阱 2: 意外拷贝
    std::vector<std::string> names = {"Alice", "Bob"};
    
    for (auto name : names) {  
        // name 是 string 的拷贝！每次循环都会拷贝
        std::cout << name.length() << std::endl;
    }
    
    // ✅ 正确做法：使用引用
    for (const auto& name : names) {
        // name 是 const string&，无拷贝
        std::cout << name.length() << std::endl;
    }
    
    // ⚠️ 陷阱 3: auto 与窄化转换
    auto narrow = 3.14;  // double
    int i = narrow;      // 隐式转换，可能丢失精度
    
    // ✅ 显式转换
    auto j = static_cast<int>(narrow);
    
    // ⚠️ 陷阱 4: 初始化列表推导
    auto list1 = {1, 2, 3};     // list1 是 std::initializer_list<int>
    auto list2{1, 2, 3};        // C++17: 也是 std::initializer_list<int>
    
    return 0;
}
```

---

## 二、范围 for 循环 (Range-based for)

### 2.1 基本语法

```cpp
#include <iostream>
#include <vector>
#include <array>

int main() {
    // ===== 基础用法 =====
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // 只读访问
    for (int n : numbers) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // 使用 auto
    for (auto n : numbers) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // ===== 修改元素 =====
    for (auto& n : numbers) {
        n *= 2;  // 修改原数组
    }
    
    // ===== 与 C 风格数组 =====
    int c_array[] = {10, 20, 30, 40, 50};
    for (int val : c_array) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    // ===== 与 std::array =====
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    for (const auto& val : arr) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 2.2 嵌入式应用示例

```cpp
#include <cstdint>
#include <array>

// ===== LED 控制示例 =====
constexpr int NUM_LEDS = 8;
std::array<uint8_t, NUM_LEDS> led_states{};

// 初始化所有 LED
void init_leds() {
    for (auto& led : led_states) {
        led = 0;  // 关闭所有 LED
    }
}

// 设置 LED 状态
void set_led(int index, bool state) {
    if (index >= 0 && index < NUM_LEDS) {
        led_states[index] = state ? 1 : 0;
    }
}

// 遍历检查 LED 状态
void check_leds() {
    int on_count = 0;
    for (const auto& led : led_states) {
        if (led) {
            ++on_count;
        }
    }
    // on_count 是点亮的 LED 数量
}

// ===== 寄存器配置数组 =====
struct RegConfig {
    uint32_t address;
    uint32_t value;
};

constexpr std::array<RegConfig, 3> gpio_init_seq{{
    {0x48000000, 0x00000001},
    {0x48000004, 0x00000002},
    {0x48000008, 0x00000004}
}};

void apply_reg_config() {
    for (const auto& config : gpio_init_seq) {
        // 写入寄存器
        volatile uint32_t* reg = 
            reinterpret_cast<volatile uint32_t*>(config.address);
        *reg = config.value;
    }
}
```

### 2.3 范围 for 与传统 for 对比

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    // ===== 传统 for 循环 =====
    // 优点：可以获取索引，可以跳过元素
    // 缺点：容易出错，代码冗长
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << "data[" << i << "] = " << data[i] << std::endl;
    }
    
    // ===== 范围 for 循环 =====
    // 优点：简洁，不易出错
    // 缺点：无法直接获取索引
    for (const auto& val : data) {
        std::cout << "value = " << val << std::endl;
    }
    
    // ===== 需要索引时的解决方案 =====
    
    // 方案 1: 手动维护索引
    size_t idx = 0;
    for (const auto& val : data) {
        std::cout << "data[" << idx << "] = " << val << std::endl;
        ++idx;
    }
    
    // 方案 2: 使用传统 for (当需要索引时)
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << "data[" << i << "] = " << data[i] << std::endl;
    }
    
    // 方案 3: C++20 ranges (如果编译器支持)
    // for (auto [i, val] : data | std::views::enumerate) { ... }
    
    return 0;
}
```

---

## 三、综合练习

### 练习 1: 类型推导练习

```cpp
// 预测以下变量的类型
#include <vector>
#include <string>

std::vector<std::string> names = {"Alice", "Bob"};

auto a = names.begin();           // 答案：std::vector<std::string>::iterator
auto b = *names.begin();          // 答案：std::string
auto c = &names;                  // 答案：std::vector<std::string>*
auto d = names[0];                // 答案：std::string
auto e = (names.size() > 0);      // 答案：bool
```

### 练习 2: 重构 C 风格代码

将以下 C 风格代码改写为现代 C++ 风格：

```cpp
// C 风格原始代码
#include <stdio.h>

int main() {
    int numbers[] = {1, 2, 3, 4, 5};
    int sum = 0;
    int i;
    
    for (i = 0; i < 5; i++) {
        sum += numbers[i];
    }
    
    printf("Sum = %d\n", sum);
    return 0;
}

// 现代 C++ 答案
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> numbers = {1, 2, 3, 4, 5};
    
    int sum = 0;
    for (const auto& num : numbers) {
        sum += num;
    }
    
    std::cout << "Sum = " << sum << std::endl;
    return 0;
}
```

### 练习 3: 嵌入式 LED 控制器

```cpp
// 任务：使用 auto 和范围 for 实现 LED 控制器
// 要求：
// 1. 使用 std::array 存储 LED 状态
// 2. 使用范围 for 初始化 LED
// 3. 使用 auto 简化类型声明

#include <array>
#include <cstdint>

class LedController {
public:
    static constexpr int NUM_LEDS = 8;
    
    LedController() {
        // 使用范围 for 初始化
        for (auto& led : led_states_) {
            led = false;
        }
    }
    
    void turn_on(int index) {
        if (index >= 0 && index < NUM_LEDS) {
            led_states_[index] = true;
        }
    }
    
    void turn_off(int index) {
        if (index >= 0 && index < NUM_LEDS) {
            led_states_[index] = false;
        }
    }
    
    int count_on() const {
        int count = 0;
        // 使用 auto 和范围 for
        for (const auto& led : led_states_) {
            if (led) {
                ++count;
            }
        }
        return count;
    }
    
private:
    std::array<bool, NUM_LEDS> led_states_;
};
```

---

## 四、编译与测试

```bash
# 编译命令
g++ -std=c++17 -Wall -Wextra -Wpedantic -o auto_test auto_type.cpp

# 运行测试
./auto_test

# 使用 clang (推荐)
clang++ -std=c++17 -Wall -Wextra -Wpedantic -o auto_test auto_type.cpp
```

---

## 五、关键要点总结

| 要点 | 说明 |
|-----|------|
| ✅ 优先使用 `const auto&` | 避免不必要的拷贝 |
| ✅ 范围 for 简化遍历 | 代码更简洁，不易出错 |
| ⚠️ 注意 `vector<bool>` 陷阱 | 返回的是值不是引用 |
| ⚠️ 注意窄化转换 | `auto` 可能隐藏类型转换问题 |
| ⚠️ 初始化列表推导 | `{}` 推导为 `std::initializer_list` |

---

## 六、下一步

完成本章节后，继续学习：
- [RAII 惯用法](./raii.md)
- [智能指针](./smart_pointers.md)
