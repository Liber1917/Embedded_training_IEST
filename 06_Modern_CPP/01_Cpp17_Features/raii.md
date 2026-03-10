# RAII (Resource Acquisition Is Initialization)

> **学习目标**：掌握 RAII 核心思想，能够手写实现 RAII 类，理解 RAII 在嵌入式系统中的应用

---

## 一、RAII 核心概念

### 1.1 什么是 RAII？

**RAII** = **R**esource **A**cquisition **I**s **I**nitialization

> **核心思想**：将资源的生命周期绑定到对象的生命周期
> - 构造函数：获取资源
> - 析构函数：释放资源

```
┌─────────────────────────────────────────────────────────────┐
│                      RAII 生命周期                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  对象创建                        对象销毁                    │
│      │                              │                       │
│      ▼                              ▼                       │
│  ┌─────────┐                  ┌─────────┐                  │
│  │ 构造函数 │                  │ 析构函数 │                  │
│  │ 获取资源 │                  │ 释放资源 │                  │
│  └─────────┘                  └─────────┘                  │
│       │                              │                       │
│       │    对象生命周期期间          │                       │
│       │    资源始终有效             │                       │
│       └───────────┬────────────────┘                       │
│                   │                                         │
│                   ▼                                         │
│          无需手动管理资源！                                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 C 风格 vs RAII 对比

```cpp
// ========== C 风格资源管理 ==========
void c_style_example() {
    FILE* file = std::fopen("data.txt", "r");
    if (!file) {
        return;  // 错误处理
    }
    
    // 使用文件...
    if (some_error_condition()) {
        std::fclose(file);  // ⚠️ 容易忘记
        return;
    }
    
    // 更多操作...
    if (another_error()) {
        // ⚠️ 又忘记关闭！内存泄漏/资源泄漏
        return;
    }
    
    std::fclose(file);  // 最终关闭
}

// ========== RAII 风格 ==========
class FileHandle {
public:
    explicit FileHandle(const char* path, const char* mode)
        : file_(std::fopen(path, mode)) {
        if (!file_) {
            throw std::runtime_error("Failed to open file");
        }
    }
    
    ~FileHandle() {
        if (file_) {
            std::fclose(file_);
        }
    }
    
    // 禁止拷贝
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    // 允许移动
    FileHandle(FileHandle&& other) noexcept : file_(other.file_) {
        other.file_ = nullptr;
    }
    
    FILE* get() const { return file_; }
    
private:
    FILE* file_;
};

void raii_example() {
    FileHandle file("data.txt", "r");  // 构造时打开
    
    // 使用文件
    if (some_error_condition()) {
        return;  // ✅ 安全！file 析构时自动关闭
    }
    
    // 更多操作...
    if (another_error()) {
        return;  // ✅ 安全！file 析构时自动关闭
    }
    
}  // ✅ file 离开作用域，自动关闭
```

---

## 二、手写 RAII 类

### 2.1 基本 RAII 模板

```cpp
#include <memory>
#include <iostream>

// ===== 通用 RAII 模板 =====
template<typename Resource, typename Deleter>
class RaiiWrapper {
public:
    // 构造函数：获取资源
    template<typename... Args>
    explicit RaiiWrapper(Args&&... args)
        : resource_(create_resource(std::forward<Args>(args)...)),
          deleter_(std::forward<Deleter>(deleter_)) {
        if (!resource_) {
            throw std::runtime_error("Failed to acquire resource");
        }
    }
    
    // 析构函数：释放资源
    ~RaiiWrapper() {
        if (resource_) {
            deleter_(resource_);
        }
    }
    
    // 禁止拷贝
    RaiiWrapper(const RaiiWrapper&) = delete;
    RaiiWrapper& operator=(const RaiiWrapper&) = delete;
    
    // 允许移动
    RaiiWrapper(RaiiWrapper&& other) noexcept
        : resource_(other.resource_),
          deleter_(other.deleter_) {
        other.resource_ = nullptr;
    }
    
    RaiiWrapper& operator=(RaiiWrapper&& other) noexcept {
        if (this != &other) {
            if (resource_) {
                deleter_(resource_);
            }
            resource_ = other.resource_;
            deleter_ = other.deleter_;
            other.resource_ = nullptr;
        }
        return *this;
    }
    
    // 访问资源
    Resource* get() const { return resource_; }
    Resource& operator*() const { return *resource_; }
    Resource* operator->() const { return resource_; }
    
    // 释放所有权
    Resource* release() {
        Resource* temp = resource_;
        resource_ = nullptr;
        return temp;
    }
    
private:
    Resource* resource_;
    Deleter deleter_;
    
    // 创建资源的辅助函数（需要特化）
    template<typename... Args>
    static Resource* create_resource(Args&&... args);
};
```

### 2.2 具体 RAII 实现示例

```cpp
#include <cstdio>
#include <pthread.h>
#include <cstdint>

// ===== 示例 1: RAII 文件句柄 =====
class File {
public:
    explicit File(const char* path, const char* mode)
        : file_(std::fopen(path, mode)) {
        if (!file_) {
            throw std::runtime_error("Failed to open: " + std::string(path));
        }
    }
    
    ~File() {
        if (file_) {
            std::fclose(file_);
        }
    }
    
    // 禁止拷贝
    File(const File&) = delete;
    File& operator=(const File&) = delete;
    
    // 允许移动
    File(File&& other) noexcept : file_(other.file_) {
        other.file_ = nullptr;
    }
    
    File& operator=(File&& other) noexcept {
        if (this != &other) {
            if (file_) std::fclose(file_);
            file_ = other.file_;
            other.file_ = nullptr;
        }
        return *this;
    }
    
    FILE* get() const { return file_; }
    bool is_open() const { return file_ != nullptr; }
    
private:
    FILE* file_;
};

// ===== 示例 2: RAII 互斥锁 =====
class MutexLock {
public:
    explicit MutexLock(pthread_mutex_t* mutex)
        : mutex_(mutex), owns_(false) {
        if (mutex_ && pthread_mutex_lock(mutex_) == 0) {
            owns_ = true;
        }
    }
    
    ~MutexLock() {
        if (owns_ && mutex_) {
            pthread_mutex_unlock(mutex_);
        }
    }
    
    // 禁止拷贝和移动
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;
    MutexLock(MutexLock&&) = delete;
    MutexLock& operator=(MutexLock&&) = delete;
    
    bool owns_lock() const { return owns_; }
    
private:
    pthread_mutex_t* mutex_;
    bool owns_;
};

// 使用示例
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

void thread_safe_function() {
    MutexLock lock(&global_mutex);  // 构造时加锁
    
    // 临界区代码
    // ... 即使这里抛出异常，析构时也会解锁
    
}  // 析构时自动解锁

// ===== 示例 3: RAII 内存对齐分配 =====
class AlignedMemory {
public:
    AlignedMemory(size_t size, size_t alignment)
        : size_(size), ptr_(nullptr) {
        #ifdef _WIN32
            ptr_ = _aligned_malloc(size, alignment);
        #else
            if (posix_memalign(&ptr_, alignment, size) != 0) {
                ptr_ = nullptr;
            }
        #endif
        
        if (!ptr_) {
            throw std::bad_alloc();
        }
    }
    
    ~AlignedMemory() {
        if (ptr_) {
            #ifdef _WIN32
                _aligned_free(ptr_);
            #else
                free(ptr_);
            #endif
        }
    }
    
    // 禁止拷贝
    AlignedMemory(const AlignedMemory&) = delete;
    AlignedMemory& operator=(const AlignedMemory&) = delete;
    
    // 允许移动
    AlignedMemory(AlignedMemory&& other) noexcept
        : size_(other.size_), ptr_(other.ptr_) {
        other.ptr_ = nullptr;
        other.size_ = 0;
    }
    
    void* get() const { return ptr_; }
    size_t size() const { return size_; }
    
private:
    size_t size_;
    void* ptr_;
};

// ===== 示例 4: RAII GPIO 配置（嵌入式） =====
class GpioConfig {
public:
    struct Config {
        uint32_t moder;
        uint32_t otyper;
        uint32_t ospeedr;
        uint32_t pupdr;
    };
    
    GpioConfig(uint32_t base_addr, const Config& config)
        : moder_(reinterpret_cast<volatile uint32_t*>(base_addr)),
          otyper_(reinterpret_cast<volatile uint32_t*>(base_addr + 0x04)),
          ospeedr_(reinterpret_cast<volatile uint32_t*>(base_addr + 0x08)),
          pupdr_(reinterpret_cast<volatile uint32_t*>(base_addr + 0x0C)),
          saved_config_{} {
        
        // 保存原始配置
        saved_config_.moder = *moder_;
        saved_config_.otyper = *otyper_;
        saved_config_.ospeedr = *ospeedr_;
        saved_config_.pupdr = *pupdr_;
        
        // 应用新配置
        *moder_ = config.moder;
        *otyper_ = config.otyper;
        *ospeedr_ = config.ospeedr;
        *pupdr_ = config.pupdr;
    }
    
    ~GpioConfig() {
        // 恢复原始配置
        *moder_ = saved_config_.moder;
        *otyper_ = saved_config_.otyper;
        *ospeedr_ = saved_config_.ospeedr;
        *pupdr_ = saved_config_.pupdr;
    }
    
    // 禁止拷贝和移动
    GpioConfig(const GpioConfig&) = delete;
    GpioConfig& operator=(const GpioConfig&) = delete;
    GpioConfig(GpioConfig&&) = delete;
    GpioConfig& operator=(GpioConfig&&) = delete;
    
private:
    volatile uint32_t* moder_;
    volatile uint32_t* otyper_;
    volatile uint32_t* ospeedr_;
    volatile uint32_t* pupdr_;
    Config saved_config_;
};
```

---

## 三、RAII 在嵌入式系统中的应用

### 3.1 中断管理

```cpp
#include <cstdint>

// ===== RAII 中断禁用/启用 =====
class InterruptGuard {
public:
    InterruptGuard() : disabled_(false) {
        // 保存当前中断状态并禁用
        // primask_ = __get_PRIMASK();
        // __disable_irq();
        disabled_ = true;
    }
    
    ~InterruptGuard() {
        if (disabled_) {
            // 恢复中断状态
            // if (!primask_) __enable_irq();
        }
    }
    
    // 禁止拷贝和移动
    InterruptGuard(const InterruptGuard&) = delete;
    InterruptGuard& operator=(const InterruptGuard&) = delete;
    InterruptGuard(InterruptGuard&&) = delete;
    InterruptGuard& operator=(InterruptGuard&&) = delete;
    
private:
    bool disabled_;
    // uint32_t primask_;
};

// 使用示例
volatile uint32_t shared_counter = 0;

void increment_counter() {
    InterruptGuard guard;  // 禁用中断
    // 临界区：安全修改共享变量
    ++shared_counter;
}  // 自动恢复中断

// ===== RAII 中断服务程序注册 =====
template<int IRQ_NUM>
class IrqHandler {
public:
    using HandlerFunc = void(*)();
    
    explicit IrqHandler(HandlerFunc handler)
        : registered_(false) {
        // 保存旧处理程序
        // old_handler_ = get_irq_handler(IRQ_NUM);
        
        // 注册新处理程序
        // set_irq_handler(IRQ_NUM, handler);
        // enable_irq(IRQ_NUM);
        registered_ = true;
    }
    
    ~IrqHandler() {
        if (registered_) {
            // 禁用中断
            // disable_irq(IRQ_NUM);
            // 恢复旧处理程序
            // set_irq_handler(IRQ_NUM, old_handler_);
        }
    }
    
    // 禁止拷贝
    IrqHandler(const IrqHandler&) = delete;
    IrqHandler& operator=(const IrqHandler&) = delete;
    
private:
    bool registered_;
    // HandlerFunc old_handler_;
};
```

### 3.2 DMA 传输管理

```cpp
#include <cstdint>
#include <functional>

// ===== RAII DMA 传输 =====
class DmaTransfer {
public:
    using CompletionCallback = std::function<void(bool)>;
    
    DmaTransfer(uint32_t stream,
                const void* src,
                void* dst,
                size_t size,
                CompletionCallback callback = nullptr)
        : stream_(stream),
          size_(size),
          callback_(callback),
          completed_(false) {
        
        // 配置 DMA
        // DMA_Config(stream_, src, dst, size);
        
        // 启动传输
        // DMA_Start(stream_);
    }
    
    ~DmaTransfer() {
        if (!completed_) {
            // 如果未完成，等待或取消
            // DMA_Abort(stream_);
        }
    }
    
    // 等待完成
    bool wait(uint32_t timeout_ms) {
        // return DMA_Wait(stream_, timeout_ms);
        completed_ = true;
        return true;
    }
    
    // 禁止拷贝
    DmaTransfer(const DmaTransfer&) = delete;
    DmaTransfer& operator=(const DmaTransfer&) = delete;
    
private:
    uint32_t stream_;
    size_t size_;
    CompletionCallback callback_;
    bool completed_;
};

// 使用示例
void dma_example() {
    uint8_t buffer[256];
    
    {
        DmaTransfer transfer(
            0,  // DMA 流
            reinterpret_cast<const void*>(0x40000000),  // 源地址
            buffer,  // 目标地址
            256,     // 大小
            [](bool success) {
                if (success) {
                    // 传输完成回调
                }
            }
        );
        
        transfer.wait(1000);  // 等待最多 1 秒
    }  // transfer 离开作用域，确保清理
}
```

### 3.3 外设电源管理

```cpp
#include <cstdint>

// ===== RAII 外设时钟使能 =====
class PeripheralClock {
public:
    enum class Peripheral {
        GPIOA, GPIOB, GPIOC,
        USART1, USART2,
        SPI1, SPI2,
        I2C1, I2C2
    };
    
    PeripheralClock(Peripheral periph)
        : periph_(periph) {
        enable_clock();
    }
    
    ~PeripheralClock() {
        disable_clock();
    }
    
    // 禁止拷贝
    PeripheralClock(const PeripheralClock&) = delete;
    PeripheralClock& operator=(const PeripheralClock&) = delete;
    
private:
    Peripheral periph_;
    
    void enable_clock() {
        // 根据外设使能对应时钟
        // RCC->AHB1ENR |= clock_mask(periph_);
    }
    
    void disable_clock() {
        // 禁用时钟以节省功耗
        // RCC->AHB1ENR &= ~clock_mask(periph_);
    }
};

// 使用示例
void use_gpio() {
    PeripheralClock gpio_clock(PeripheralClock::Peripheral::GPIOA);
    
    // 使用 GPIOA，时钟自动使能
    // GPIOA->MODER = 0x55555555;
    
}  // 离开作用域，时钟自动禁用（省电）
```

---

## 四、RAII 与智能指针的关系

```cpp
#include <memory>

// RAII 是智能指针的基础理念
// 智能指针是 RAII 的具体实现

// ===== unique_ptr 是 RAII 的实现 =====
template<typename T>
class SimpleUniquePtr {
public:
    explicit SimpleUniquePtr(T* ptr = nullptr) : ptr_(ptr) {}
    
    ~SimpleUniquePtr() {
        delete ptr_;  // RAII: 析构时释放
    }
    
    // 禁止拷贝
    SimpleUniquePtr(const SimpleUniquePtr&) = delete;
    SimpleUniquePtr& operator=(const SimpleUniquePtr&) = delete;
    
    // 允许移动
    SimpleUniquePtr(SimpleUniquePtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    T* get() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    
private:
    T* ptr_;
};

// ===== shared_ptr 也是 RAII 的实现 =====
// 只是增加了引用计数机制
```

---

## 五、实战练习

### 练习 1: 实现 RAII 锁守卫

```cpp
#include <mutex>

// 任务：实现一个通用的锁守卫类
template<typename Mutex>
class LockGuard {
public:
    explicit LockGuard(Mutex& m) : mutex_(&m) {
        mutex_->lock();
    }
    
    ~LockGuard() {
        mutex_->unlock();
    }
    
    // 禁止拷贝和移动
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;
    
private:
    Mutex* mutex_;
};

// 使用示例
std::mutex mtx;
int shared_data = 0;

void safe_increment() {
    LockGuard<std::mutex> lock(mtx);
    ++shared_data;
}  // 自动解锁
```

### 练习 2: 实现 RAII 作用域退出

```cpp
#include <functional>

// 任务：实现一个通用的作用域退出守卫
class ScopeExit {
public:
    using Func = std::function<void()>;
    
    explicit ScopeExit(Func func) 
        : func_(std::move(func)), 
          dismissed_(false) {}
    
    ~ScopeExit() {
        if (!dismissed_ && func_) {
            func_();
        }
    }
    
    // 取消执行
    void dismiss() { dismissed_ = true; }
    
    // 禁止拷贝
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    
private:
    Func func_;
    bool dismissed_;
};

// 辅助宏
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define SCOPE_EXIT(code) \
    ScopeExit CONCAT(_scope_exit_, __LINE__)([&]() { code; })

// 使用示例
void example() {
    FILE* f = std::fopen("test.txt", "w");
    if (!f) return;
    
    SCOPE_EXIT({
        std::fclose(f);
        std::cout << "File closed" << std::endl;
    });
    
    // 使用文件...
    // 无论函数如何退出，文件都会被关闭
}
```

### 练习 3: 实现 RAII 事务

```cpp
#include <vector>

// 任务：实现一个简单的事务守卫
template<typename RollbackFunc>
class TransactionGuard {
public:
    explicit TransactionGuard(RollbackFunc rollback)
        : rollback_(std::move(rollback)),
          committed_(false) {}
    
    ~TransactionGuard() {
        if (!committed_ && rollback_) {
            rollback_();
        }
    }
    
    void commit() { committed_ = true; }
    
    // 禁止拷贝
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
    
private:
    RollbackFunc rollback_;
    bool committed_;
};

// 使用示例
std::vector<int> data = {1, 2, 3};
auto backup = data;

void risky_operation() {
    TransactionGuard guard([&]() {
        // 回滚：恢复原始数据
        data = backup;
        std::cout << "Transaction rolled back" << std::endl;
    });
    
    // 修改数据
    data.push_back(4);
    data.push_back(5);
    
    // 可能抛出异常的操作
    // if (error) throw std::runtime_error("Error!");
    
    guard.commit();  // 提交事务
}
```

---

## 六、编译与测试

```bash
# 编译命令
g++ -std=c++17 -Wall -Wextra -Wpedantic -pthread -o raii_test raii.cpp

# 运行测试
./raii_test

# 内存检查
valgrind --leak-check=full ./raii_test
```

---

## 七、关键要点总结

| 要点 | 说明 |
|-----|------|
| ✅ RAII 核心：构造获取，析构释放 | 资源生命周期绑定对象生命周期 |
| ✅ 禁止拷贝，允许移动 | 避免资源重复释放 |
| ✅ 智能指针是 RAII 的实现 | `unique_ptr`/`shared_ptr` |
| ✅ 嵌入式应用广泛 | 中断管理、DMA、外设时钟 |
| ⚠️ 注意异常安全 | 确保析构函数不抛出异常 |
| ⚠️ 避免在 RAII 对象中存储裸指针引用 | 可能导致悬空指针 |

---

## 八、下一步

完成本章节后，继续学习：
- [智能指针](./smart_pointers.md)
- [移动语义](./move_semantics.md)
