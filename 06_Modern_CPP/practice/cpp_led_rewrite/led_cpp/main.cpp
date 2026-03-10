// main.cpp - LED 控制器测试
#include "led.hpp"
#include <iostream>
#include <thread>
#include <chrono>

// 声明模拟 GPIO 函数
extern uint32_t get_simulated_gpio();

int main() {
    std::cout << "=== LED Controller Test ===" << std::endl;
    std::cout << "(Using modern C++ implementation)" << std::endl;
    
    LedController leds;
    
    // 设置状态回调
    leds.set_state_callback([](int index, bool state) {
        std::cout << "  [Callback] LED " << index 
                  << (state ? " turned ON" : " turned OFF") << std::endl;
    });
    
    // ===== 测试 1: 基本 LED 控制 =====
    {
        std::cout << "\n[Test 1] Basic LED Control" << std::endl;
        std::cout << "===========================" << std::endl;
        
        std::cout << "Turning on LEDs one by one..." << std::endl;
        for (int i = 0; i < LedController::NUM_LEDS; ++i) {
            leds.set(i, true);
        }
        
        std::cout << "LEDs on: " << leds.count_on() << std::endl;
        std::cout << "All on: " << (leds.all_on() ? "yes" : "no") << std::endl;
        std::cout << "GPIO ODR: 0x" << std::hex << get_simulated_gpio() << std::dec << std::endl;
    }
    
    // ===== 测试 2: 切换操作 =====
    {
        std::cout << "\n[Test 2] Toggle Operation" << std::endl;
        std::cout << "==========================" << std::endl;
        
        std::cout << "Toggling LEDs 0, 2, 4, 6..." << std::endl;
        leds.toggle(0);
        leds.toggle(2);
        leds.toggle(4);
        leds.toggle(6);
        
        std::cout << "LEDs on after toggle: " << leds.count_on() << std::endl;
        std::cout << "GPIO ODR: 0x" << std::hex << get_simulated_gpio() << std::dec << std::endl;
    }
    
    // ===== 测试 3: 批量操作 =====
    {
        std::cout << "\n[Test 3] Batch Operations" << std::endl;
        std::cout << "=========================" << std::endl;
        
        std::cout << "All off..." << std::endl;
        leds.all_off();
        std::cout << "LEDs on: " << leds.count_on() << std::endl;
        
        std::cout << "All on..." << std::endl;
        leds.all_on();
        std::cout << "LEDs on: " << leds.count_on() << std::endl;
        
        std::cout << "Set multiple (mask = 0xAA)..." << std::endl;
        leds.set_multiple(0xAA);
        std::cout << "LEDs on: " << leds.count_on() << std::endl;
    }
    
    // ===== 测试 4: 范围 for 遍历 =====
    {
        std::cout << "\n[Test 4] Range-based For Loop" << std::endl;
        std::cout << "==============================" << std::endl;
        
        std::cout << "Current LED states: ";
        for (const auto& led : leds) {
            std::cout << (led ? "1" : "0");
        }
        std::cout << std::endl;
        
        std::cout << "LEDs that are ON: ";
        int index = 0;
        for (const auto& led : leds) {
            if (led) {
                std::cout << index << " ";
            }
            ++index;
        }
        std::cout << std::endl;
    }
    
    // ===== 测试 5: 算法库使用 =====
    {
        std::cout << "\n[Test 5] STL Algorithms" << std::endl;
        std::cout << "========================" << std::endl;
        
        std::cout << "any_on(): " << (leds.any_on() ? "yes" : "no") << std::endl;
        std::cout << "all_on(): " << (leds.all_on() ? "yes" : "no") << std::endl;
        std::cout << "count_on(): " << leds.count_on() << std::endl;
    }
    
    // ===== 测试 6: 移动语义 =====
    {
        std::cout << "\n[Test 6] Move Semantics" << std::endl;
        std::cout << "========================" << std::endl;
        
        LedController leds2 = std::move(leds);
        std::cout << "After move, leds2 count_on(): " << leds2.count_on() << std::endl;
    }
    
    // ===== 测试 7: 边界检查 =====
    {
        std::cout << "\n[Test 7] Boundary Check" << std::endl;
        std::cout << "========================" << std::endl;
        
        std::cout << "Trying to set invalid index -1..." << std::endl;
        leds.set(-1, true);  // 应该被忽略
        std::cout << "Trying to set invalid index 100..." << std::endl;
        leds.set(100, true);  // 应该被忽略
        std::cout << "count_on() after invalid access: " << leds.count_on() << std::endl;
    }
    
    std::cout << "\n=== All Tests Completed ===" << std::endl;
    
    return 0;
}
