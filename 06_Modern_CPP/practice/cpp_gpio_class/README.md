# cpp_gpio_class - GPIO 类 RAII 封装

> **目标**：使用 RAII 和智能指针封装 GPIO 资源，确保资源正确管理

---

## 📋 项目概述

本练习实现一个完整的 GPIO 类封装，应用以下现代 C++ 特性：
- RAII 资源管理
- 智能指针
- 移动语义
- 异常安全

---

## 📁 项目结构

```
cpp_gpio_class/
├── README.md                  # 本文件
├── gpio.hpp                   # GPIO 类头文件
├── gpio.cpp                   # GPIO 类实现（可选）
└── main.cpp                   # 测试代码
```

---

## 🎯 设计要求

### 核心功能
1. **RAII 初始化**：构造函数配置 GPIO，析构函数恢复状态
2. **智能指针管理**：使用 `unique_ptr` 管理资源
3. **移动语义**：支持所有权的转移
4. **异常安全**：构造失败时正确清理

### GPIO 功能
- 方向配置（输入/输出）
- 上拉/下拉配置
- 读写操作
- 中断配置（可选）

---

## 📝 参考实现

### gpio.hpp

```cpp
// gpio.hpp - RAII GPIO 封装
#ifndef GPIO_HPP
#define GPIO_HPP

#include <memory>
#include <cstdint>
#include <stdexcept>

// ===== GPIO 配置 =====
enum class GpioDirection {
    Input,
    Output
};

enum class GpioPull {
    None,
    Up,
    Down
};

enum class GpioSpeed {
    Low,
    Medium,
    High,
    VeryHigh
};

enum class GpioError {
    Ok,
    InvalidPin,
    InitFailed,
    AccessFailed
};

// ===== GPIO 寄存器结构（以 STM32 为例）=====
struct GpioRegisters {
    volatile uint32_t MODER;    // 模式寄存器
    volatile uint32_t OTYPER;   // 输出类型
    volatile uint32_t OSPEEDR;  // 速度寄存器
    volatile uint32_t PUPDR;    // 上拉/下拉
    volatile uint32_t IDR;      // 输入数据
    volatile uint32_t ODR;      // 输出数据
    volatile uint32_t BSRR;     // 置位/复位
    volatile uint32_t LCKR;     // 锁定寄存器
    volatile uint32_t AFRL;     // 复用功能低
    volatile uint32_t AFRH;     // 复用功能高
};

// ===== GPIO 配置 =====
struct GpioConfig {
    GpioDirection direction = GpioDirection::Input;
    GpioPull pull = GpioPull::None;
    GpioSpeed speed = GpioSpeed::Medium;
    bool open_drain = false;
};

// ===== RAII GPIO 类 =====
class GpioPin {
public:
    // 引脚编号
    enum class Pin {
        PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7,
        PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15,
        PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7,
        PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15,
        // 可根据需要添加更多引脚
    };
    
    // ===== 构造与析构 =====
    
    // 构造函数：RAII 初始化
    explicit GpioPin(Pin pin, const GpioConfig& config = GpioConfig{});
    
    // 析构函数：恢复默认状态
    ~GpioPin();
    
    // 禁止拷贝（资源独占）
    GpioPin(const GpioPin&) = delete;
    GpioPin& operator=(const GpioPin&) = delete;
    
    // 允许移动
    GpioPin(GpioPin&& other) noexcept;
    GpioPin& operator=(GpioPin&& other) noexcept;
    
    // ===== 状态查询 =====
    
    Pin pin() const noexcept { return pin_; }
    bool is_valid() const noexcept { return registers_ != nullptr; }
    GpioDirection direction() const noexcept { return config_.direction; }
    
    // ===== 配置方法 =====
    
    void set_direction(GpioDirection dir);
    void set_pull(GpioPull pull);
    void set_speed(GpioSpeed speed);
    void set_open_drain(bool enable);
    
    void configure(const GpioConfig& config);
    
    // ===== 读写操作 =====
    
    // 输出
    void set(bool value);
    void set_high() { set(true); }
    void set_low() { set(false); }
    void toggle();
    bool read() const;
    
    // 原子操作
    void set_atomic(bool value);
    
    // ===== 原始访问（用于特殊场景）=====
    
    GpioRegisters* registers() const noexcept { return registers_; }
    int pin_number() const noexcept { return pin_number_; }
    
    // ===== 静态方法 =====
    
    // 批量配置多个引脚
    template<size_t N>
    static void configure_multiple(const Pin (&pins)[N], 
                                   const GpioConfig& config) {
        for (const auto& pin : pins) {
            GpioPin gpio(pin, config);
            // gpio 离开作用域时自动清理
        }
    }
    
private:
    // 成员变量
    Pin pin_;
    int port_;
    int pin_number_;
    GpioRegisters* registers_;
    GpioConfig config_;
    bool owned_;  // 是否拥有资源
    
    // 保存的原始配置（用于恢复）
    uint32_t saved_moder_;
    uint32_t saved_otyper_;
    uint32_t saved_ospeedr_;
    uint32_t saved_pupdr_;
    
    // 内部方法
    void init_registers();
    void restore_registers();
    static GpioRegisters* get_registers_for_pin(Pin pin);
    static void enable_port_clock(int port);
};

// ===== 自定义删除器（用于 unique_ptr 包装）=====
struct GpioDeleter {
    void operator()(GpioPin* pin) const {
        delete pin;
    }
};

// 便捷类型
using GpioPinPtr = std::unique_ptr<GpioPin, GpioDeleter>;

// ===== 工厂函数 =====
GpioPinPtr make_gpio(GpioPin::Pin pin, const GpioConfig& config = GpioConfig{});

// ===== GPIO 端口类（管理一组引脚）=====
class GpioPort {
public:
    explicit GpioPort(int port_num);
    ~GpioPort();
    
    // 禁止拷贝
    GpioPort(const GpioPort&) = delete;
    GpioPort& operator=(const GpioPort&) = delete;
    
    // 引脚访问
    GpioPin pin(int num);
    
    // 批量操作
    void write(uint16_t value);
    uint16_t read() const;
    void set_mask(uint16_t mask, uint16_t value);
    
private:
    int port_;
    GpioRegisters* registers_;
    bool clock_enabled_;
};

#endif // GPIO_HPP
```

### gpio.cpp

```cpp
// gpio.cpp - RAII GPIO 实现
#include "gpio.hpp"
#include <array>

// GPIO 基地址（以 STM32F4 为例）
constexpr uint32_t GPIOA_BASE = 0x48000000;
constexpr uint32_t GPIOB_BASE = 0x48000400;
constexpr uint32_t GPIOC_BASE = 0x48000800;

// RCC 寄存器（时钟控制）
constexpr uint32_t RCC_BASE = 0x40023800;
constexpr uint32_t RCC_AHB1ENR = RCC_BASE + 0x30;

// ===== GpioPin 实现 =====

GpioPin::GpioPin(Pin pin, const GpioConfig& config)
    : pin_(pin)
    , port_(static_cast<int>(pin) / 16)
    , pin_number_(static_cast<int>(pin) % 16)
    , registers_(nullptr)
    , config_(config)
    , owned_(true)
    , saved_moder_(0)
    , saved_otyper_(0)
    , saved_ospeedr_(0)
    , saved_pupdr_(0) 
{
    // 验证引脚有效性
    if (pin_number_ < 0 || pin_number_ > 15) {
        throw std::invalid_argument("Invalid GPIO pin");
    }
    
    // 获取寄存器基地址
    registers_ = get_registers_for_pin(pin);
    if (!registers_) {
        throw std::runtime_error("Invalid GPIO port");
    }
    
    // 使能端口时钟
    enable_port_clock(port_);
    
    // 保存原始配置
    saved_moder_ = registers_->MODER;
    saved_otyper_ = registers_->OTYPER;
    saved_ospeedr_ = registers_->OSPEEDR;
    saved_pupdr_ = registers_->PUPDR;
    
    // 应用新配置
    init_registers();
}

GpioPin::~GpioPin() {
    if (owned_ && registers_) {
        restore_registers();
    }
}

GpioPin::GpioPin(GpioPin&& other) noexcept
    : pin_(other.pin_)
    , port_(other.port_)
    , pin_number_(other.pin_number_)
    , registers_(other.registers_)
    , config_(other.config_)
    , owned_(other.owned_)
    , saved_moder_(other.saved_moder_)
    , saved_otyper_(other.saved_otyper_)
    , saved_ospeedr_(other.saved_ospeedr_)
    , saved_pupdr_(other.saved_pupdr_)
{
    // 转移所有权
    other.registers_ = nullptr;
    other.owned_ = false;
}

GpioPin& GpioPin::operator=(GpioPin&& other) noexcept {
    if (this != &other) {
        // 清理当前资源
        if (owned_ && registers_) {
            restore_registers();
        }
        
        // 转移所有权
        pin_ = other.pin_;
        port_ = other.port_;
        pin_number_ = other.pin_number_;
        registers_ = other.registers_;
        config_ = other.config_;
        owned_ = other.owned_;
        saved_moder_ = other.saved_moder_;
        saved_otyper_ = other.saved_otyper_;
        saved_ospeedr_ = other.saved_ospeedr_;
        saved_pupdr_ = other.saved_pupdr_;
        
        // 清空源对象
        other.registers_ = nullptr;
        other.owned_ = false;
    }
    return *this;
}

void GpioPin::set_direction(GpioDirection dir) {
    if (!registers_) return;
    
    config_.direction = dir;
    
    uint32_t mode = (dir == GpioDirection::Output) ? 1 : 0;
    uint32_t mask = 0b11u << (pin_number_ * 2);
    
    registers_->MODER = (registers_->MODER & ~mask) | (mode << (pin_number_ * 2));
}

void GpioPin::set_pull(GpioPull pull) {
    if (!registers_) return;
    
    config_.pull = pull;
    
    uint32_t pupd = static_cast<uint32_t>(pull);
    uint32_t mask = 0b11u << (pin_number_ * 2);
    
    registers_->PUPDR = (registers_->PUPDR & ~mask) | (pupd << (pin_number_ * 2));
}

void GpioPin::set_speed(GpioSpeed speed) {
    if (!registers_) return;
    
    config_.speed = speed;
    
    uint32_t spd = static_cast<uint32_t>(speed);
    uint32_t mask = 0b11u << (pin_number_ * 2);
    
    registers_->OSPEEDR = (registers_->OSPEEDR & ~mask) | (spd << (pin_number_ * 2));
}

void GpioPin::set_open_drain(bool enable) {
    if (!registers_) return;
    
    config_.open_drain = enable;
    
    uint32_t mask = 1u << pin_number_;
    if (enable) {
        registers_->OTYPER |= mask;
    } else {
        registers_->OTYPER &= ~mask;
    }
}

void GpioPin::configure(const GpioConfig& config) {
    config_ = config;
    init_registers();
}

void GpioPin::set(bool value) {
    if (!registers_) return;
    
    if (value) {
        registers_->ODR |= (1u << pin_number_);
    } else {
        registers_->ODR &= ~(1u << pin_number_);
    }
}

void GpioPin::toggle() {
    if (!registers_) return;
    
    registers_->ODR ^= (1u << pin_number_);
}

bool GpioPin::read() const {
    if (!registers_) return false;
    
    return (registers_->IDR & (1u << pin_number_)) != 0;
}

void GpioPin::set_atomic(bool value) {
    if (!registers_) return;
    
    // 使用 BSRR 实现原子操作
    if (value) {
        registers_->BSRR = (1u << pin_number_);
    } else {
        registers_->BSRR = (1u << (pin_number_ + 16));
    }
}

void GpioPin::init_registers() {
    if (!registers_) return;
    
    set_direction(config_.direction);
    set_pull(config_.pull);
    set_speed(config_.speed);
    set_open_drain(config_.open_drain);
}

void GpioPin::restore_registers() {
    if (!registers_) return;
    
    registers_->MODER = saved_moder_;
    registers_->OTYPER = saved_otyper_;
    registers_->OSPEEDR = saved_ospeedr_;
    registers_->PUPDR = saved_pupdr_;
}

GpioRegisters* GpioPin::get_registers_for_pin(Pin pin) {
    int port = static_cast<int>(pin) / 16;
    
    switch (port) {
        case 0: return reinterpret_cast<GpioRegisters*>(GPIOA_BASE);
        case 1: return reinterpret_cast<GpioRegisters*>(GPIOB_BASE);
        case 2: return reinterpret_cast<GpioRegisters*>(GPIOC_BASE);
        default: return nullptr;
    }
}

void GpioPin::enable_port_clock(int port) {
    volatile auto* rcc = reinterpret_cast<volatile uint32_t*>(RCC_AHB1ENR);
    *rcc |= (1u << port);
}

// ===== 工厂函数 =====

GpioPinPtr make_gpio(GpioPin::Pin pin, const GpioConfig& config) {
    return GpioPinPtr(new GpioPin(pin, config));
}

// ===== GpioPort 实现 =====

GpioPort::GpioPort(int port_num)
    : port_(port_num)
    , registers_(nullptr)
    , clock_enabled_(false)
{
    switch (port_num) {
        case 0: registers_ = reinterpret_cast<GpioRegisters*>(GPIOA_BASE); break;
        case 1: registers_ = reinterpret_cast<GpioRegisters*>(GPIOB_BASE); break;
        case 2: registers_ = reinterpret_cast<GpioRegisters*>(GPIOC_BASE); break;
        default: throw std::invalid_argument("Invalid port number");
    }
    
    GpioPin::enable_port_clock(port_num);
    clock_enabled_ = true;
}

GpioPort::~GpioPort() {
    // 可选：禁用时钟
    // volatile auto* rcc = reinterpret_cast<volatile uint32_t*>(RCC_AHB1ENR);
    // *rcc &= ~(1u << port_);
}

GpioPin GpioPort::pin(int num) {
    if (num < 0 || num > 15) {
        throw std::invalid_argument("Invalid pin number");
    }
    
    auto pin = static_cast<GpioPin::Pin>(port_ * 16 + num);
    return GpioPin(pin);
}

void GpioPort::write(uint16_t value) {
    if (registers_) {
        registers_->ODR = (registers_->ODR & 0xFFFF0000) | value;
    }
}

uint16_t GpioPort::read() const {
    return registers_ ? static_cast<uint16_t>(registers_->IDR & 0xFFFF) : 0;
}

void GpioPort::set_mask(uint16_t mask, uint16_t value) {
    if (!registers_) return;
    
    uint32_t odr = registers_->ODR;
    odr = (odr & ~mask) | (value & mask);
    registers_->ODR = odr;
}
```

### main.cpp

```cpp
// main.cpp - GPIO 类测试
#include "gpio.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== GPIO RAII Test ===" << std::endl;
    
    try {
        // ===== 测试 1: 基本 RAII =====
        {
            std::cout << "\n[Test 1] Basic RAII" << std::endl;
            
            GpioConfig config;
            config.direction = GpioDirection::Output;
            config.pull = GpioPull::Up;
            
            GpioPin led(GpioPin::Pin::PA5, config);
            
            std::cout << "Pin valid: " << led.is_valid() << std::endl;
            std::cout << "Pin number: " << led.pin_number() << std::endl;
            
            // LED 闪烁
            for (int i = 0; i < 5; ++i) {
                led.set_high();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                led.set_low();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
        }  // led 离开作用域，自动恢复 GPIO 配置
        
        // ===== 测试 2: 移动语义 =====
        {
            std::cout << "\n[Test 2] Move Semantics" << std::endl;
            
            GpioPin pin1(GpioPin::Pin::PA6);
            GpioPin pin2 = std::move(pin1);
            
            std::cout << "pin1 valid: " << pin1.is_valid() << std::endl;  // false
            std::cout << "pin2 valid: " << pin2.is_valid() << std::endl;  // true
            
            pin2.set_high();
        }
        
        // ===== 测试 3: 工厂函数 =====
        {
            std::cout << "\n[Test 3] Factory Function" << std::endl;
            
            GpioConfig config;
            config.direction = GpioDirection::Input;
            config.pull = GpioPull::Down;
            
            auto pin = make_gpio(GpioPin::Pin::PA7, config);
            
            std::cout << "Pin valid: " << pin->is_valid() << std::endl;
            std::cout << "Pin value: " << pin->read() << std::endl;
            
        }  // unique_ptr 自动释放
        
        // ===== 测试 4: GPIO 端口 =====
        {
            std::cout << "\n[Test 4] GPIO Port" << std::endl;
            
            GpioPort port(0);  // GPIOA
            
            // 配置多个引脚为输出
            for (int i = 0; i < 8; ++i) {
                auto pin = port.pin(i);
                pin.set_direction(GpioDirection::Output);
            }
            
            // 批量写入
            port.write(0x00FF);
            std::cout << "Port write: 0x00FF" << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            port.write(0x0000);
        }
        
        // ===== 测试 5: 异常安全 =====
        {
            std::cout << "\n[Test 5] Exception Safety" << std::endl;
            
            try {
                // 尝试无效引脚
                GpioPin invalid(static_cast<GpioPin::Pin>(100));
            } catch (const std::exception& e) {
                std::cout << "Caught exception: " << e.what() << std::endl;
            }
        }
        
        std::cout << "\n=== All Tests Passed ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## 🔧 编译与测试

```bash
# 进入目录
cd cpp_gpio_class

# 编译
g++ -std=c++17 -Wall -Wextra -Wpedantic -o gpio_test main.cpp gpio.cpp

# 运行测试
./gpio_test

# 内存检查
valgrind --leak-check=full ./gpio_test
```

---

## ✅ 验收检查清单

### RAII 实现
- [ ] 构造函数获取资源
- [ ] 析构函数释放资源
- [ ] 保存并恢复原始配置
- [ ] 禁止拷贝，允许移动

### 智能指针
- [ ] 工厂函数返回 `unique_ptr`
- [ ] 自定义删除器（如需要）
- [ ] 正确的所有权转移

### 异常安全
- [ ] 构造失败时正确清理
- [ ] 使用 try-catch 处理错误
- [ ] 资源不会泄漏

### 代码质量
- [ ] 无编译警告
- [ ] 有完整注释
- [ ] 命名规范一致

---

## 📚 关键要点

### RAII 优势

| C 风格 | C++ RAII |
|-------|---------|
| 手动初始化/反初始化 | 自动管理 |
| 容易忘记清理 | 析构函数保证清理 |
| 异常不安全 | 异常安全 |
| 资源泄漏风险 | 无资源泄漏 |

### 移动语义应用

```cpp
// 转移 GPIO 所有权
GpioPin create_pin() {
    GpioPin pin(GpioPin::Pin::PA5);
    return pin;  // 移动或 RVO
}

void use_pin() {
    GpioPin pin1(GpioPin::Pin::PA5);
    GpioPin pin2 = std::move(pin1);  // pin1 不再有效
}
```

---

## 🔗 参考资源

- [RAII 惯用法](../../01_Cpp17_Features/raii.md)
- [智能指针](../../01_Cpp17_Features/smart_pointers.md)
- [移动语义](../../01_Cpp17_Features/move_semantics.md)

---

*返回：[实战练习首页](../README.md)*
