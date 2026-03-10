// gpio.cpp - RAII GPIO 实现
#include "gpio.hpp"

// ===== GpioPin 实现 =====

GpioPin::GpioPin() noexcept
    : pin_(Pin::PA0)
    , port_(0)
    , pin_number_(0)
    , registers_(nullptr)
    , config_{}
    , owned_(false)
    , saved_moder_(0)
    , saved_otyper_(0)
    , saved_ospeedr_(0)
    , saved_pupdr_(0) 
{
}

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
    // 注意：在实际嵌入式系统中，这里应该返回真实的硬件地址
    // 以下是示例地址，需要根据具体 MCU 修改
    int port = static_cast<int>(pin) / 16;
    
    // 这些地址需要根据目标平台修改
    // 对于测试，返回 nullptr 并在测试代码中覆盖此方法
    (void)port;
    return nullptr;
}

void GpioPin::enable_port_clock(int port) {
    // 注意：在实际嵌入式系统中，这里应该操作 RCC 寄存器
    // 对于测试，此方法可以在测试代码中覆盖
    (void)port;
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
    // 注意：这些地址需要根据目标平台修改
    switch (port_num) {
        case 0: /* registers_ = reinterpret_cast<GpioRegisters*>(GPIOA_BASE); */ break;
        case 1: /* registers_ = reinterpret_cast<GpioRegisters*>(GPIOB_BASE); */ break;
        case 2: /* registers_ = reinterpret_cast<GpioRegisters*>(GPIOC_BASE); */ break;
        default: 
            throw std::invalid_argument("Invalid port number");
    }
    
    // 使能时钟
    // GpioPin::enable_port_clock(port_num);
    clock_enabled_ = true;
}

GpioPort::~GpioPort() {
    // 可选：禁用时钟以节省功耗
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
