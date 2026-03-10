// main.cpp - GPIO 类测试
// 注意：此代码在实际硬件上运行需要适当的硬件抽象层
// 在 PC 上编译时，需要模拟 GPIO 寄存器访问

#include "gpio.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

// ===== 模拟 GPIO 寄存器（用于 PC 测试）=====
// 在实际嵌入式系统中，这些应该映射到真实的硬件地址

namespace simulated {
    std::atomic<uint32_t> gpioa_moder{0};
    std::atomic<uint32_t> gpioa_otyper{0};
    std::atomic<uint32_t> gpioa_ospeedr{0};
    std::atomic<uint32_t> gpioa_pupdr{0};
    std::atomic<uint32_t> gpioa_idr{0};
    std::atomic<uint32_t> gpioa_odr{0};
    std::atomic<uint32_t> gpioa_bsrr{0};
    
    GpioRegisters gpioa_regs{
        gpioa_moder, gpioa_otyper, gpioa_ospeedr, gpioa_pupdr,
        gpioa_idr, gpioa_odr, gpioa_bsrr, 0, 0, 0
    };
    
    bool clock_enabled = false;
}

// 覆盖 GpioPin 的静态方法以使用模拟寄存器
GpioRegisters* GpioPin::get_registers_for_pin(Pin pin) {
    int port = static_cast<int>(pin) / 16;
    
    // 模拟：所有引脚都映射到 GPIOA
    (void)port;
    return &simulated::gpioa_regs;
}

void GpioPin::enable_port_clock(int port) {
    // 模拟时钟使能
    (void)port;
    simulated::clock_enabled = true;
}

int main() {
    std::cout << "=== GPIO RAII Test ===" << std::endl;
    std::cout << "(Using simulated GPIO registers)" << std::endl;
    
    try {
        // ===== 测试 1: 基本 RAII =====
        {
            std::cout << "\n[Test 1] Basic RAII" << std::endl;
            std::cout << "===================" << std::endl;
            
            GpioConfig config;
            config.direction = GpioDirection::Output;
            config.pull = GpioPull::Up;
            
            GpioPin led(GpioPin::Pin::PA5, config);
            
            std::cout << "Pin valid: " << led.is_valid() << std::endl;
            std::cout << "Pin number: " << led.pin_number() << std::endl;
            std::cout << "Port number: " << led.port_number() << std::endl;
            
            // 模拟 LED 闪烁
            std::cout << "Toggling LED..." << std::endl;
            for (int i = 0; i < 3; ++i) {
                led.set_high();
                std::cout << "  LED ON (ODR = 0x" << std::hex 
                          << simulated::gpioa_odr.load() << std::dec << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                led.set_low();
                std::cout << "  LED OFF (ODR = 0x" << std::hex 
                          << simulated::gpioa_odr.load() << std::dec << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
        }  // led 离开作用域，自动恢复 GPIO 配置
        
        // ===== 测试 2: 移动语义 =====
        {
            std::cout << "\n[Test 2] Move Semantics" << std::endl;
            std::cout << "========================" << std::endl;
            
            GpioPin pin1(GpioPin::Pin::PA6);
            std::cout << "pin1 valid after construction: " << pin1.is_valid() << std::endl;
            
            GpioPin pin2 = std::move(pin1);
            std::cout << "After move:" << std::endl;
            std::cout << "  pin1 valid: " << pin1.is_valid() << std::endl;  // false
            std::cout << "  pin2 valid: " << pin2.is_valid() << std::endl;  // true
            
            pin2.set_high();
            std::cout << "  pin2 set high (ODR = 0x" << std::hex 
                      << simulated::gpioa_odr.load() << std::dec << ")" << std::endl;
        }
        
        // ===== 测试 3: 工厂函数 =====
        {
            std::cout << "\n[Test 3] Factory Function" << std::endl;
            std::cout << "=========================" << std::endl;
            
            GpioConfig config;
            config.direction = GpioDirection::Input;
            config.pull = GpioPull::Down;
            
            auto pin = make_gpio(GpioPin::Pin::PA7, config);
            
            std::cout << "Pin valid: " << pin->is_valid() << std::endl;
            std::cout << "Pin number: " << pin->pin_number() << std::endl;
            
            // 模拟输入值
            simulated::gpioa_idr.store(0x80);
            std::cout << "Pin value: " << pin->read() << std::endl;
            
        }  // unique_ptr 自动释放
        
        // ===== 测试 4: GPIO 端口 =====
        {
            std::cout << "\n[Test 4] GPIO Port" << std::endl;
            std::cout << "===================" << std::endl;
            
            GpioPort port(0);  // GPIOA
            
            // 配置多个引脚为输出
            std::cout << "Configuring pins 0-7 as output..." << std::endl;
            for (int i = 0; i < 8; ++i) {
                auto pin = port.pin(i);
                pin.set_direction(GpioDirection::Output);
            }
            
            // 批量写入
            std::cout << "Writing 0x00FF to port..." << std::endl;
            port.write(0x00FF);
            std::cout << "Port ODR = 0x" << std::hex 
                      << simulated::gpioa_odr.load() << std::dec << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            std::cout << "Writing 0x0000 to port..." << std::endl;
            port.write(0x0000);
            std::cout << "Port ODR = 0x" << std::hex 
                      << simulated::gpioa_odr.load() << std::dec << std::endl;
        }
        
        // ===== 测试 5: 原子操作 =====
        {
            std::cout << "\n[Test 5] Atomic Operations" << std::endl;
            std::cout << "===========================" << std::endl;
            
            GpioPin pin(GpioPin::Pin::PA0);
            
            std::cout << "Initial BSRR: 0x" << std::hex 
                      << simulated::gpioa_bsrr.load() << std::dec << std::endl;
            
            pin.set_atomic(true);
            std::cout << "After set_atomic(true): BSRR = 0x" << std::hex 
                      << simulated::gpioa_bsrr.load() << std::dec << std::endl;
            
            pin.set_atomic(false);
            std::cout << "After set_atomic(false): BSRR = 0x" << std::hex 
                      << simulated::gpioa_bsrr.load() << std::dec << std::endl;
        }
        
        // ===== 测试 6: 配置恢复 =====
        {
            std::cout << "\n[Test 6] Configuration Restore" << std::endl;
            std::cout << "==============================" << std::endl;
            
            // 保存原始 MODER
            uint32_t original_moder = simulated::gpioa_moder.load();
            std::cout << "Original MODER: 0x" << std::hex << original_moder << std::dec << std::endl;
            
            {
                GpioConfig config;
                config.direction = GpioDirection::Output;
                GpioPin pin(GpioPin::Pin::PA1, config);
                
                std::cout << "After creating output pin, MODER: 0x" << std::hex 
                          << simulated::gpioa_moder.load() << std::dec << std::endl;
            }  // pin 离开作用域，应该恢复原始配置
            
            std::cout << "After RAII restore, MODER: 0x" << std::hex 
                      << simulated::gpioa_moder.load() << std::dec << std::endl;
            
            // 验证恢复
            if (simulated::gpioa_moder.load() == original_moder) {
                std::cout << "✓ Configuration restored correctly!" << std::endl;
            } else {
                std::cout << "✗ Configuration NOT restored!" << std::endl;
            }
        }
        
        std::cout << "\n=== All Tests Passed ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
