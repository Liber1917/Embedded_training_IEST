# Lambda 表达式

> **学习目标**：掌握 Lambda 表达式语法，理解捕获列表的使用，能够在嵌入式系统中使用 Lambda 作为回调函数

---

## 一、为什么需要 Lambda？

### 1.1 回调函数的问题

```cpp
// ========== C 风格回调函数 ==========
#include <functional>

// 问题：需要单独定义函数
void print_callback(int value) {
    std::cout << "Value: " << value << std::endl;
}

void process_data(std::function<void(int)> callback) {
    callback(42);
}

void c_style_example() {
    process_data(print_callback);  // 需要单独定义函数
}

// 问题：无法携带上下文
int g_multiplier = 2;

void multiply_callback(int value) {
    std::cout << "Result: " << value * g_multiplier << std::endl;
}
// 依赖全局变量，不安全！

// ========== C++ Lambda 风格 ==========
void lambda_example() {
    int multiplier = 2;
    
    // Lambda：就地定义，携带上下文
    auto callback = [multiplier](int value) {
        std::cout << "Result: " << value * multiplier << std::endl;
    };
    
    process_data(callback);
    
    // 或者直接传递
    process_data([multiplier](int value) {
        std::cout << "Result: " << value * multiplier << std::endl;
    });
}
```

---

## 二、Lambda 基本语法

### 2.1 语法结构

```
┌─────────────────────────────────────────────────────────────┐
│                    Lambda 表达式语法                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [捕获列表] (参数列表) mutable 异常说明 -> 返回类型 { 函数体 }  │
│     │          │         │       │        │          │      │
│     │          │         │       │        │          └── 必选 │
│     │          │         │       │        └── 可选          │
│     │          │         │       └── 可选                   │
│     │          │         └── 可选                           │
│     │          └── 可选（可省略）                            │
│     └── 必选                                                 │
│                                                             │
│  最简单形式：[] { /* 代码 */ }                               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 基础示例

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    // ===== 最简单的 Lambda =====
    auto hello = [] {
        std::cout << "Hello, Lambda!" << std::endl;
    };
    hello();
    
    // ===== 带参数的 Lambda =====
    auto add = [](int a, int b) {
        return a + b;
    };
    std::cout << "2 + 3 = " << add(2, 3) << std::endl;
    
    // ===== 自动推导返回类型 =====
    auto multiply = [](int a, int b) {
        return a * b;  // 返回类型自动推导为 int
    };
    
    // ===== 显式指定返回类型 =====
    auto divide = [](double a, double b) -> double {
        return a / b;
    };
    
    // ===== 作为函数参数 =====
    std::vector<int> numbers = {5, 2, 8, 1, 9};
    
    // 使用 Lambda 作为排序谓词
    std::sort(numbers.begin(), numbers.end(),
        [](int a, int b) {
            return a > b;  // 降序排序
        }
    );
    
    // 使用 Lambda 作为查找谓词
    auto it = std::find_if(numbers.begin(), numbers.end(),
        [](int n) {
            return n % 2 == 0;  // 查找偶数
        }
    );
    
    if (it != numbers.end()) {
        std::cout << "First even: " << *it << std::endl;
    }
    
    return 0;
}
```

---

## 三、捕获列表详解

### 3.1 捕获方式

```cpp
#include <iostream>
#include <memory>

int main() {
    int local_var = 42;
    int& ref_var = local_var;
    
    // ===== 空捕获 =====
    auto empty = [] {
        // 不能访问外部变量
        // std::cout << local_var;  // 错误！
    };
    
    // ===== 值捕获 [=] =====
    auto by_value = [=] {
        std::cout << local_var << std::endl;  // 42
        // local_var = 100;  // 错误！const 副本
    };
    
    // ===== 引用捕获 [&] =====
    auto by_ref = [&] {
        std::cout << local_var << std::endl;  // 42
        local_var = 100;  // OK！修改原变量
    };
    
    // ===== 混合捕获 =====
    int x = 1, y = 2, z = 3;
    
    // 默认值捕获，y 用引用捕获
    auto mixed1 = [=, &y] {
        // x, z 是值捕获（const 副本）
        // y 是引用捕获
        y = 100;
    };
    
    // 默认引用捕获，x 用值捕获
    auto mixed2 = [&, x] {
        // x 是值捕获（const 副本）
        // y, z 是引用捕获
    };
    
    // ===== 命名捕获 =====
    auto named = [local_var] {
        // 只捕获 local_var（值捕获）
        std::cout << local_var << std::endl;
    };
    
    auto named_ref = [&ref_var] {
        // 只捕获 ref_var（引用捕获）
        ref_var = 200;
    };
    
    // ===== C++14 初始化捕获 =====
    auto init_capture = [value = 42, ptr = std::make_unique<int>(100)] {
        std::cout << "value = " << value << std::endl;
        std::cout << "*ptr = " << *ptr << std::endl;
    };
    
    // ===== C++14 移动捕获 =====
    auto move_capture = [ptr = std::move(std::make_unique<int>(42))] {
        std::cout << "*ptr = " << *ptr << std::endl;
    };
    
    return 0;
}
```

### 3.2 捕获方式对比表

| 捕获方式 | 语法 | 特点 | 使用场景 |
|---------|------|-----|---------|
| 空捕获 | `[]` | 不捕获任何变量 | 独立函数 |
| 值捕获 | `[=]` | 捕获所有使用的变量（const 副本） | 不修改外部变量 |
| 引用捕获 | `[&]` | 捕获所有使用的变量（引用） | 需要修改外部变量 |
| 混合捕获 | `[=, &x]` | 默认值捕获，x 引用捕获 | 大部分值捕获，少数引用 |
| 混合捕获 | `[&, x]` | 默认引用捕获，x 值捕获 | 大部分引用，少数值捕获 |
| 命名捕获 | `[x, &y]` | 显式指定每个变量 | 精确控制 |
| this 捕获 | `[this]` | 捕获当前对象 | 成员函数中 |
| 初始化捕获 | `[x = expr]` | C++14，创建新变量 | 移动捕获、转换类型 |

---

## 四、mutable 与 Lambda 状态

### 4.1 mutable 关键字

```cpp
#include <iostream>

int main() {
    int counter = 0;
    
    // ===== 值捕获默认是 const =====
    auto const_lambda = [counter] {
        // counter++;  // 错误！const 副本
        std::cout << counter << std::endl;
    };
    
    // ===== 使用 mutable 修改副本 =====
    auto mutable_lambda = [counter]() mutable {
        counter++;  // OK！修改自己的副本
        std::cout << "Inside: " << counter << std::endl;
    };
    
    mutable_lambda();  // Inside: 1
    mutable_lambda();  // Inside: 1（每次都是新副本）
    std::cout << "Outside: " << counter << std::endl;  // Outside: 0
    
    // ===== 引用捕获不需要 mutable =====
    auto ref_lambda = [&counter] {
        counter++;  // OK！修改原变量
    };
    
    ref_lambda();
    ref_lambda();
    std::cout << "Outside: " << counter << std::endl;  // Outside: 2
    
    return 0;
}
```

### 4.2 Lambda 状态与可调用次数

```cpp
#include <iostream>
#include <functional>

// ===== Lambda 的类型 =====
// 每个 Lambda 有唯一的匿名类型

void lambda_types() {
    int x = 42;
    
    // 无捕获 Lambda 可以转换为函数指针
    auto no_capture = [](int a, int b) { return a + b; };
    int (*func_ptr)(int, int) = no_capture;
    
    // 有捕获 Lambda 不能转换为函数指针
    auto with_capture = [x](int a) { return a + x; };
    // int (*bad_ptr)(int) = with_capture;  // 错误！
    
    // 但可以使用 std::function
    std::function<int(int)> func = with_capture;
}

// ===== Lambda 作为状态机 =====
auto create_counter() {
    return [count = 0]() mutable {
        return ++count;
    };
}

void state_machine_example() {
    auto counter = create_counter();
    
    std::cout << counter() << std::endl;  // 1
    std::cout << counter() << std::endl;  // 2
    std::cout << counter() << std::endl;  // 3
}
```

---

## 五、嵌入式应用：回调函数

### 5.1 定时器回调

```cpp
#include <functional>
#include <cstdint>

// ===== 模拟定时器类 =====
class Timer {
public:
    using Callback = std::function<void()>;
    
    static void start(uint32_t interval_ms, Callback callback) {
        // 注册回调
        // 实际实现会设置硬件定时器
        callbacks_.push_back({interval_ms, callback});
    }
    
    // 模拟定时器中断
    static void tick() {
        for (auto& entry : callbacks_) {
            if (++entry.counter >= entry.interval_ms) {
                entry.counter = 0;
                entry.callback();
            }
        }
    }
    
private:
    struct Entry {
        uint32_t interval_ms;
        Callback callback;
        uint32_t counter = 0;
    };
    static std::vector<Entry> callbacks_;
};

std::vector<Timer::Entry> Timer::callbacks_;

// ===== 使用示例 =====
void timer_example() {
    int blink_count = 0;
    bool led_state = false;
    
    // 使用 Lambda 作为定时器回调
    Timer::start(1000, [&blink_count, &led_state]() {
        // 每秒切换 LED 状态
        led_state = !led_state;
        blink_count++;
        
        if (blink_count >= 10) {
            // 10 次后停止
            // Timer::stop();
        }
    });
}
```

### 5.2 中断处理回调

```cpp
#include <functional>
#include <array>

// ===== 中断服务程序注册 =====
class InterruptManager {
public:
    using Handler = std::function<void()>;
    
    static void register_irq(int irq_num, Handler handler) {
        if (irq_num >= 0 && irq_num < MAX_IRQ) {
            handlers_[irq_num] = std::move(handler);
        }
    }
    
    // 模拟中断触发
    static void trigger_irq(int irq_num) {
        if (irq_num >= 0 && irq_num < MAX_IRQ && handlers_[irq_num]) {
            handlers_[irq_num]();
        }
    }
    
private:
    static constexpr int MAX_IRQ = 16;
    static std::array<Handler, MAX_IRQ> handlers_;
};

std::array<InterruptManager::Handler, 16> InterruptManager::handlers_;

// ===== 使用示例 =====
void irq_example() {
    int button_press_count = 0;
    
    // 注册按钮中断处理
    InterruptManager::register_irq(0, [&button_press_count]() {
        button_press_count++;
        // 处理按钮按下事件
    });
    
    // 注册串口中断处理
    InterruptManager::register_irq(1, []() {
        // 处理串口接收
        // uint8_t data = UART_Receive();
    });
}
```

### 5.3 GPIO 事件回调

```cpp
#include <functional>
#include <vector>

// ===== GPIO 事件系统 =====
class GpioEvent {
public:
    enum class Edge {
        RISING,
        FALLING,
        BOTH
    };
    
    using Callback = std::function<void()>;
    
    static void attach_interrupt(int pin, Edge edge, Callback callback) {
        events_.push_back({pin, edge, std::move(callback)});
    }
    
    // 模拟 GPIO 变化
    static void simulate_change(int pin, bool value) {
        for (auto& event : events_) {
            if (event.pin == pin) {
                // 检查边沿
                bool should_trigger = false;
                
                switch (event.edge) {
                    case Edge::RISING:
                        should_trigger = value;
                        break;
                    case Edge::FALLING:
                        should_trigger = !value;
                        break;
                    case Edge::BOTH:
                        should_trigger = true;
                        break;
                }
                
                if (should_trigger && event.callback) {
                    event.callback();
                }
            }
        }
    }
    
private:
    struct Event {
        int pin;
        Edge edge;
        Callback callback;
    };
    static std::vector<Event> events_;
};

std::vector<GpioEvent> GpioEvent::events_;

// ===== 使用示例 =====
void gpio_event_example() {
    // 上升沿触发
    GpioEvent::attach_interrupt(13, GpioEvent::Edge::RISING, []() {
        // LED 开启
    });
    
    // 下降沿触发
    GpioEvent::attach_interrupt(13, GpioEvent::Edge::FALLING, []() {
        // LED 关闭
    });
}
```

---

## 六、Lambda 与 std::function

### 6.1 std::function 基础

```cpp
#include <functional>
#include <iostream>
#include <vector>

// ===== std::function 作为通用包装器 =====
void function_example() {
    // 普通函数
    auto add = [](int a, int b) { return a + b; };
    
    // std::function 包装
    std::function<int(int, int)> func = add;
    
    std::cout << func(2, 3) << std::endl;  // 5
    
    // 也可以包装普通函数
    std::function<void(const char*)> print = std::printf;
    print("Hello!\n");
}

// ===== 作为类成员 =====
class Task {
public:
    using Callback = std::function<void(int)>;
    
    explicit Task(Callback callback) 
        : callback_(std::move(callback)) {}
    
    void execute(int value) {
        if (callback_) {
            callback_(value);
        }
    }
    
private:
    Callback callback_;
};

void task_example() {
    int result = 0;
    
    Task task([&result](int value) {
        result = value * 2;
    });
    
    task.execute(21);
    std::cout << "Result: " << result << std::endl;  // 42
}

// ===== 存储多个回调 =====
class EventSystem {
public:
    using Listener = std::function<void(const std::string&)>;
    
    void subscribe(Listener listener) {
        listeners_.push_back(std::move(listener));
    }
    
    void emit(const std::string& event) {
        for (auto& listener : listeners_) {
            listener(event);
        }
    }
    
private:
    std::vector<Listener> listeners_;
};

void event_system_example() {
    EventSystem events;
    
    events.subscribe([](const std::string& e) {
        std::cout << "Listener 1: " << e << std::endl;
    });
    
    events.subscribe([&](const std::string& e) {
        std::cout << "Listener 2: " << e << std::endl;
    });
    
    events.emit("Hello, Events!");
}
```

### 6.2 性能考虑

```cpp
#include <functional>
#include <chrono>
#include <iostream>

// ===== Lambda vs std::function 性能对比 =====

// 直接 Lambda（最快）
void direct_lambda() {
    auto func = [](int x) { return x * 2; };
    volatile int result = 0;
    for (int i = 0; i < 1000000; ++i) {
        result += func(i);
    }
}

// std::function 包装（有开销）
void function_wrapper() {
    std::function<int(int)> func = [](int x) { return x * 2; };
    volatile int result = 0;
    for (int i = 0; i < 1000000; ++i) {
        result += func(i);
    }
}

void performance_test() {
    auto start = std::chrono::high_resolution_clock::now();
    direct_lambda();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Direct Lambda: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    function_wrapper();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "std::function: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms" << std::endl;
}

// 建议：
// - 性能关键代码：直接使用 Lambda 或模板
// - 需要多态/存储：使用 std::function
```

---

## 七、常见陷阱

### 7.1 陷阱与解决方案

```cpp
#include <iostream>
#include <vector>
#include <functional>

// ⚠️ 陷阱 1: 悬空引用
void trap1() {
    std::function<void()> func;
    
    {
        int x = 42;
        func = [&x] {
            std::cout << x << std::endl;  // 危险！
        };
    }  // x 离开作用域
    
    func();  // 未定义行为！访问已销毁的变量
}

// ✅ 正确：值捕获
void fix1() {
    std::function<void()> func;
    
    {
        int x = 42;
        func = [x] {  // 值捕获
            std::cout << x << std::endl;
        };
    }
    
    func();  // OK
}

// ⚠️ 陷阱 2: this 指针悬空
class Trap2 {
public:
    std::function<void()> get_callback() {
        return [this] {
            // 使用 this->member
            process();
        };
    }
    
    void process() {}
};

void trap2() {
    std::function<void()> callback;
    
    {
        Trap2 obj;
        callback = obj.get_callback();
    }  // obj 销毁
    
    callback();  // 未定义行为！this 悬空
}

// ✅ 正确：使用 shared_ptr
class Fix2 : public std::enable_shared_from_this<Fix2> {
public:
    std::function<void()> get_callback() {
        auto self = shared_from_this();
        return [self] {
            self->process();
        };
    }
    
    void process() {}
};

// ⚠️ 陷阱 3: 循环引用
class Trap3;

class Owner {
public:
    std::function<void()> callback;
    ~Owner() { std::cout << "Owner destroyed" << std::endl; }
};

class Trap3 : public std::enable_shared_from_this<Trap3> {
public:
    std::shared_ptr<Owner> owner;
    
    void setup() {
        owner = std::make_shared<Owner>();
        owner->callback = [this]() {
            // this 是 shared_ptr，导致循环引用
            process();
        };
    }
    
    void process() {}
    ~Trap3() { std::cout << "Trap3 destroyed" << std::endl; }
};

void trap3() {
    auto obj = std::make_shared<Trap3>();
    obj->setup();
    // obj 和 owner->callback 互相引用，无法释放
}

// ✅ 正确：使用 weak_ptr
class Fix3 : public std::enable_shared_from_this<Fix3> {
public:
    std::shared_ptr<Owner> owner;
    
    void setup() {
        owner = std::make_shared<Owner>();
        auto weak_this = weak_from_this();
        owner->callback = [weak_this]() {
            if (auto self = weak_this.lock()) {
                self->process();
            }
        };
    }
    
    void process() {}
};

// ⚠️ 陷阱 4: 意外拷贝
void trap4() {
    std::vector<int> data(1000000, 42);
    
    // 值捕获会拷贝整个 vector
    auto bad = [data] {  // 大拷贝！
        return data[0];
    };
    
    // ✅ 正确：引用捕获或指针
    auto good = [&data] {
        return data[0];
    };
}
```

---

## 八、实战练习

### 练习 1: 实现命令模式

```cpp
#include <functional>
#include <vector>
#include <memory>

// 任务：使用 Lambda 实现命令模式
class Command {
public:
    using Action = std::function<void()>;
    using UndoAction = std::function<void()>;
    
    Command(Action action, UndoAction undo)
        : action_(std::move(action)), undo_(std::move(undo)) {}
    
    void execute() { action_(); }
    void undo() { undo_(); }
    
private:
    Action action_;
    UndoAction undo_;
};

class CommandHistory {
public:
    void execute(Command cmd) {
        cmd.execute();
        history_.push_back(std::move(cmd));
    }
    
    void undo() {
        if (!history_.empty()) {
            history_.back().undo();
            history_.pop_back();
        }
    }
    
private:
    std::vector<Command> history_;
};

// 使用示例
void command_example() {
    int value = 0;
    CommandHistory history;
    
    history.execute(Command(
        [&value]() { value += 10; },
        [&value]() { value -= 10; }
    ));
    
    history.undo();
}
```

### 练习 2: 实现观察者模式

```cpp
#include <functional>
#include <vector>
#include <algorithm>

// 任务：使用 Lambda 实现观察者模式
template<typename T>
class Observable {
public:
    using Observer = std::function<void(const T&)>;
    using Id = size_t;
    
    Id subscribe(Observer observer) {
        Id id = next_id_++;
        observers_.push_back({id, std::move(observer)});
        return id;
    }
    
    void unsubscribe(Id id) {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [id](const auto& entry) { return entry.id == id; }),
            observers_.end()
        );
    }
    
    void notify(const T& value) {
        for (const auto& entry : observers_) {
            entry.observer(value);
        }
    }
    
private:
    struct Entry {
        Id id;
        Observer observer;
    };
    std::vector<Entry> observers_;
    Id next_id_ = 0;
};

// 使用示例
void observable_example() {
    Observable<int> observable;
    
    auto id1 = observable.subscribe([](int v) {
        std::cout << "Observer 1: " << v << std::endl;
    });
    
    auto id2 = observable.subscribe([](int v) {
        std::cout << "Observer 2: " << v << std::endl;
    });
    
    observable.notify(42);
    
    observable.unsubscribe(id1);
    observable.notify(100);  // 只有 Observer 2 收到
}
```

---

## 九、编译与测试

```bash
# 编译命令
g++ -std=c++17 -Wall -Wextra -Wpedantic -o lambda_test lambda.cpp

# 运行测试
./lambda_test

# 性能测试
./lambda_test --benchmark
```

---

## 十、关键要点总结

| 要点 | 说明 |
|-----|------|
| ✅ Lambda 是匿名函数 | 可就地定义，携带上下文 |
| ✅ 捕获列表决定变量访问方式 | `[=]` 值捕获，`[&]` 引用捕获 |
| ✅ mutable 允许修改值捕获的副本 | 默认是 const |
| ✅ std::function 可存储 Lambda | 但有运行时开销 |
| ⚠️ 注意悬空引用 | 不要捕获局部变量的引用 |
| ⚠️ 注意循环引用 | 使用 `weak_ptr` 打破循环 |
| ⚠️ 性能敏感代码避免 std::function | 使用模板或直接 Lambda |

---

## 十一、下一步

完成本章节后，继续学习：
- [实战练习](../practice/README.md)
- [CS106L 笔记](../02_CS106L_Notes/README.md)
