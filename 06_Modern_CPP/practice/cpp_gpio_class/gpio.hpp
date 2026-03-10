// gpio.hpp - RAII GPIO 封装
#ifndef GPIO_HPP
#define GPIO_HPP

#include <memory>
#include <cstdint>
#include <stdexcept>
#include <cstddef>

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
        PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7,
        PC8, PC9, PC10, PC11, PC12, PC13, PC14, PC15,
    };
    
    // ===== 构造与析构 =====
    
    // 默认构造函数（不初始化）
    GpioPin() noexcept;
    
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
    int port_number() const noexcept { return port_; }
    int pin_number() const noexcept { return pin_number_; }
    
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
    
    // ===== 静态方法 =====
    
    // 批量配置多个引脚
    template<size_t N>
    static void configure_multiple(const Pin (&pins)[N], 
                                   const GpioConfig& config) {
        for (const auto& p : pins) {
            GpioPin gpio(p, config);
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
