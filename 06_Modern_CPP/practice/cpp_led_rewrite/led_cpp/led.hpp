// led.hpp - 现代 C++ LED 控制
#ifndef LED_HPP
#define LED_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <algorithm>

class LedController {
public:
    // LED 数量
    static constexpr int NUM_LEDS = 8;
    
    // 状态变化回调类型
    using StateCallback = std::function<void(int, bool)>;
    
    // ===== 构造与析构 =====
    
    // 构造函数 - RAII 初始化
    LedController();
    
    // 析构函数
    ~LedController() = default;
    
    // 禁止拷贝
    LedController(const LedController&) = delete;
    LedController& operator=(const LedController&) = delete;
    
    // 允许移动
    LedController(LedController&&) noexcept = default;
    LedController& operator=(LedController&&) noexcept = default;
    
    // ===== LED 控制 =====
    
    // 设置 LED 状态
    void set(int index, bool state);
    
    // 获取 LED 状态
    bool get(int index) const;
    
    // 切换 LED 状态
    void toggle(int index);
    
    // 全部关闭
    void all_off();
    
    // 全部开启
    void all_on();
    
    // ===== 状态查询 =====
    
    // 点亮点亮的 LED 数量
    int count_on() const;
    
    // 是否有 LED 点亮
    bool any_on() const;
    
    // 是否所有 LED 都点亮
    bool all_on() const;
    
    // 获取所有状态
    const std::array<bool, NUM_LEDS>& states() const { return states_; }
    
    // ===== 回调设置 =====
    
    void set_state_callback(StateCallback callback);
    
    // ===== 批量操作 =====
    
    // 设置多个 LED 状态
    void set_multiple(uint8_t mask);
    
    // 反转指定掩码的 LED
    void invert(uint8_t mask);
    
    // ===== 迭代器支持 =====
    
    using iterator = std::array<bool, NUM_LEDS>::iterator;
    using const_iterator = std::array<bool, NUM_LEDS>::const_iterator;
    
    iterator begin() { return states_.begin(); }
    iterator end() { return states_.end(); }
    const_iterator begin() const { return states_.begin(); }
    const_iterator end() const { return states_.end(); }
    const_iterator cbegin() const { return states_.cbegin(); }
    const_iterator cend() const { return states_.cend(); }
    
private:
    std::array<bool, NUM_LEDS> states_;
    StateCallback state_callback_;
    
    // 硬件操作（模拟）
    static void write_gpio(uint32_t value);
    static uint32_t read_gpio();
    
    // 通知回调
    void notify_state_change(int index, bool state);
};

#endif // LED_HPP
