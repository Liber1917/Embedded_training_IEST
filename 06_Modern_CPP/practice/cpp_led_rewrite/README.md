# cpp_led_rewrite - LED 控制 C++ 重写

> **目标**：将 C 风格的 LED 控制代码重写为现代 C++ 实现

---

## 📋 项目概述

本练习将原始的 C 风格 LED 控制代码重写为现代 C++ 实现，应用以下特性：
- 类封装
- RAII 资源管理
- auto 类型推导
- 范围 for 循环

---

## 📁 项目结构

```
cpp_led_rewrite/
├── README.md                  # 本文件
├── led_c/                     # 原始 C 代码
│   ├── led.c                  # LED 驱动实现
│   └── led.h                  # LED 驱动头文件
└── led_cpp/                   # C++ 重写
    ├── led.hpp                # LED 类头文件
    ├── led.cpp                # LED 类实现
    └── main.cpp               # 测试代码
```

---

## 📖 原始 C 代码

### led.h

```c
// led.h - C 风格 LED 控制
#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_LEDS 8

// LED 状态数组
extern uint8_t led_states[NUM_LEDS];

// 初始化 LED
void led_init(void);

// 设置 LED 状态
void led_set(int index, bool state);

// 获取 LED 状态
bool led_get(int index);

// 切换 LED 状态
void led_toggle(int index);

// 全部关闭
void led_all_off(void);

// 点亮点亮的 LED 数量
int led_count_on(void);

#endif // LED_H
```

### led.c

```c
// led.c - C 风格 LED 控制实现
#include "led.h"
#include <string.h>

// 全局状态数组
uint8_t led_states[NUM_LEDS] = {0};

// 假设的硬件寄存器
#define GPIO_ODR  (*(volatile uint32_t*)0x48000014)

void led_init(void) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        led_states[i] = 0;
    }
    GPIO_ODR = 0;  // 关闭所有 LED
}

void led_set(int index, bool state) {
    if (index >= 0 && index < NUM_LEDS) {
        led_states[index] = state ? 1 : 0;
        // 更新硬件
        if (state) {
            GPIO_ODR |= (1 << index);
        } else {
            GPIO_ODR &= ~(1 << index);
        }
    }
}

bool led_get(int index) {
    if (index >= 0 && index < NUM_LEDS) {
        return led_states[index] != 0;
    }
    return false;
}

void led_toggle(int index) {
    if (index >= 0 && index < NUM_LEDS) {
        led_set(index, !led_get(index));
    }
}

void led_all_off(void) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        led_states[i] = 0;
    }
    GPIO_ODR = 0;
}

int led_count_on(void) {
    int count = 0;
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        if (led_states[i]) {
            count++;
        }
    }
    return count;
}
```

---

## 💡 C++ 重写要求

### 基础要求
1. 使用类封装 LED 控制逻辑
2. 使用 `std::array` 替代 C 数组
3. 使用范围 for 循环
4. 使用 `auto` 类型推导

### 进阶要求
1. 使用 RAII 管理 GPIO 资源
2. 添加移动语义支持
3. 使用 `constexpr` 优化

### 挑战要求
1. 使用 Lambda 作为状态变化回调
2. 支持 LED 动画效果
3. 线程安全设计

---

## 📝 C++ 参考答案

### led.hpp

```cpp
// led.hpp - 现代 C++ LED 控制
#ifndef LED_HPP
#define LED_HPP

#include <array>
#include <cstdint>
#include <functional>

class LedController {
public:
    // LED 数量
    static constexpr int NUM_LEDS = 8;
    
    // 状态变化回调类型
    using StateCallback = std::function<void(int, bool)>;
    
    // 构造函数 - RAII 初始化
    LedController();
    
    // 禁止拷贝
    LedController(const LedController&) = delete;
    LedController& operator=(const LedController&) = delete;
    
    // 允许移动
    LedController(LedController&&) noexcept = default;
    LedController& operator=(LedController&&) noexcept = default;
    
    // LED 控制
    void set(int index, bool state);
    bool get(int index) const;
    void toggle(int index);
    void all_off();
    
    // 状态查询
    int count_on() const;
    bool any_on() const;
    bool all_on() const;
    
    // 回调设置
    void set_state_callback(StateCallback callback);
    
    // 批量操作
    template<typename Func>
    void for_each_led(Func func) {
        for (auto& led : states_) {
            func(led);
        }
    }
    
    template<typename Func>
    void for_each_led(Func func) const {
        for (const auto& led : states_) {
            func(led);
        }
    }
    
private:
    std::array<bool, NUM_LEDS> states_;
    StateCallback state_callback_;
    
    // 硬件操作
    static void write_gpio(uint32_t value);
    static uint32_t read_gpio();
};

#endif // LED_HPP
```

### led.cpp

```cpp
// led.cpp - 现代 C++ LED 控制实现
#include "led.hpp"
#include <algorithm>

// GPIO 寄存器地址（示例）
constexpr uint32_t GPIO_ODR_ADDR = 0x48000014;

LedController::LedController() 
    : states_{}, state_callback_{} {
    // 使用范围 for 初始化
    for (auto& led : states_) {
        led = false;
    }
    write_gpio(0);
}

void LedController::set(int index, bool state) {
    if (index < 0 || index >= NUM_LEDS) {
        return;
    }
    
    if (states_[index] != state) {
        states_[index] = state;
        
        // 更新硬件
        uint32_t odr = read_gpio();
        if (state) {
            odr |= (1u << index);
        } else {
            odr &= ~(1u << index);
        }
        write_gpio(odr);
        
        // 调用回调
        if (state_callback_) {
            state_callback_(index, state);
        }
    }
}

bool LedController::get(int index) const {
    if (index < 0 || index >= NUM_LEDS) {
        return false;
    }
    return states_[index];
}

void LedController::toggle(int index) {
    set(index, !get(index));
}

void LedController::all_off() {
    // 使用算法库
    std::fill(states_.begin(), states_.end(), false);
    write_gpio(0);
    
    // 通知所有 LED 关闭
    if (state_callback_) {
        for (int i = 0; i < NUM_LEDS; ++i) {
            state_callback_(i, false);
        }
    }
}

int LedController::count_on() const {
    // 使用算法库和 Lambda
    return std::count_if(states_.begin(), states_.end(),
        [](bool on) { return on; });
}

bool LedController::any_on() const {
    return std::any_of(states_.begin(), states_.end(),
        [](bool on) { return on; });
}

bool LedController::all_on() const {
    return std::all_of(states_.begin(), states_.end(),
        [](bool on) { return on; });
}

void LedController::set_state_callback(StateCallback callback) {
    state_callback_ = std::move(callback);
}

void LedController::write_gpio(uint32_t value) {
    // 实际硬件操作
    volatile auto* odr = reinterpret_cast<volatile uint32_t*>(GPIO_ODR_ADDR);
    *odr = value;
}

uint32_t LedController::read_gpio() {
    volatile auto* odr = reinterpret_cast<volatile uint32_t*>(GPIO_ODR_ADDR);
    return *odr;
}
```

### main.cpp

```cpp
// main.cpp - LED 控制器测试
#include "led.hpp"
#include <iostream>

int main() {
    LedController leds;
    
    // 设置状态回调
    leds.set_state_callback([](int index, bool state) {
        std::cout << "LED " << index 
                  << (state ? " ON" : " OFF") << std::endl;
    });
    
    // 测试基本功能
    std::cout << "=== LED Control Test ===" << std::endl;
    
    // 逐个点亮
    for (int i = 0; i < LedController::NUM_LEDS; ++i) {
        leds.set(i, true);
    }
    
    std::cout << "LEDs on: " << leds.count_on() << std::endl;
    std::cout << "All on: " << (leds.all_on() ? "yes" : "no") << std::endl;
    
    // 切换
    leds.toggle(0);
    leds.toggle(2);
    
    std::cout << "After toggle, LEDs on: " << leds.count_on() << std::endl;
    
    // 全部关闭
    leds.all_off();
    std::cout << "All off, LEDs on: " << leds.count_on() << std::endl;
    
    // 使用 for_each_led
    std::cout << "Setting even LEDs..." << std::endl;
    leds.for_each_led([&leds](bool& led, int index) {
        // 注意：需要修改 for_each_led 支持索引
    });
    
    return 0;
}
```

---

## 🔧 编译与测试

```bash
# 进入目录
cd cpp_led_rewrite/led_cpp

# 编译
g++ -std=c++17 -Wall -Wextra -Wpedantic -o led_test main.cpp led.cpp

# 运行测试
./led_test

# 内存检查
valgrind --leak-check=full ./led_test
```

---

## ✅ 验收检查清单

### 代码质量
- [ ] 使用 `std::array` 替代 C 数组
- [ ] 使用范围 for 循环
- [ ] 使用 `auto` 类型推导
- [ ] 类封装合理

### 现代 C++ 特性
- [ ] 使用 `constexpr` 定义常量
- [ ] 使用 `std::function` 作为回调
- [ ] 使用 `std::move` 转移所有权
- [ ] 使用算法库（`std::count_if` 等）

### 代码规范
- [ ] 无编译警告
- [ ] 有完整注释
- [ ] 命名规范一致

---

## 📚 学习要点

### C vs C++ 对比

| C 风格 | C++ 风格 | 优势 |
|-------|---------|-----|
| `uint8_t led_states[8]` | `std::array<bool, 8>` | 类型安全、边界检查 |
| `for (int i = 0; i < 8; i++)` | `for (const auto& led : states_)` | 简洁、不易出错 |
| 全局函数 | 类成员函数 | 封装、状态管理 |
| 函数指针回调 | `std::function` | 灵活、可携带状态 |

---

## 🔗 下一步

完成本练习后，继续：
- [GPIO 类 RAII 封装](../cpp_gpio_class/README.md)
