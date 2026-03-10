// led.cpp - 现代 C++ LED 控制实现
#include "led.hpp"
#include <iostream>

// GPIO 寄存器地址（示例 - 实际使用时需要修改）
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
        notify_state_change(index, state);
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
            notify_state_change(i, false);
        }
    }
}

void LedController::all_on() {
    // 使用算法库
    std::fill(states_.begin(), states_.end(), true);
    write_gpio(0xFF);
    
    // 通知所有 LED 开启
    if (state_callback_) {
        for (int i = 0; i < NUM_LEDS; ++i) {
            notify_state_change(i, true);
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

void LedController::set_multiple(uint8_t mask) {
    for (int i = 0; i < NUM_LEDS; ++i) {
        bool state = (mask & (1 << i)) != 0;
        set(i, state);
    }
}

void LedController::invert(uint8_t mask) {
    for (int i = 0; i < NUM_LEDS; ++i) {
        if (mask & (1 << i)) {
            toggle(i);
        }
    }
}

void LedController::notify_state_change(int index, bool state) {
    if (state_callback_) {
        state_callback_(index, state);
    }
}

// ===== 硬件操作（模拟）=====

namespace {
    // 模拟 GPIO 寄存器（用于测试）
    uint32_t simulated_gpio_odr = 0;
}

void LedController::write_gpio(uint32_t value) {
    // 实际硬件操作：
    // volatile auto* odr = reinterpret_cast<volatile uint32_t*>(GPIO_ODR_ADDR);
    // *odr = value;
    
    // 模拟操作（用于测试）
    simulated_gpio_odr = value;
}

uint32_t LedController::read_gpio() {
    // 实际硬件操作：
    // volatile auto* odr = reinterpret_cast<volatile uint32_t*>(GPIO_ODR_ADDR);
    // return *odr;
    
    // 模拟操作（用于测试）
    return simulated_gpio_odr;
}

// 获取模拟寄存器值（用于测试）
uint32_t get_simulated_gpio() {
    return simulated_gpio_odr;
}
