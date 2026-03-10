# 06_Modern_CPP - 现代 C++ 嵌入式开发

> **模块目标**：掌握 C++17/20 核心特性，理解 RAII、智能指针等现代 C++ 理念，能够用 C++ 重写现有 C 项目

---

## 📋 学习路线图

```
┌─────────────────────────────────────────────────────────────────┐
│                    14 天 现 代 C++ 学 习 路 径                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  第 1-2 天          第 3-5 天          第 6-8 天          第 9-14 天  │
│  ┌─────────┐      ┌─────────┐      ┌─────────┐      ┌─────────┐ │
│  │ 基础语法 │  →   │ RAII    │  →   │ 移动语义 │  →   │ 项目实战 │ │
│  │ 回顾    │      │ 智能指针 │      │         │      │         │ │
│  └─────────┘      └─────────┘      └─────────┘      └─────────┘ │
│       ↓                ↓                ↓                ↓       │
│  • auto 类型       • RAII 概念      • 左值/右值       • LED 重写  │
│  • 范围 for        • unique_ptr     • 移动构造        • GPIO 类  │
│  • 默认参数        • shared_ptr     • std::move       • 串口封装 │
│                    • weak_ptr                                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📁 模块结构

```
06_Modern_CPP/
├── README.md                  # 本文件 - 模块导航
│
├── 01_Cpp17_Features/         # C++17/20 核心特性详解
│   ├── README.md              # 特性概览
│   ├── auto_type.md           # auto 类型推导
│   ├── smart_pointers.md      # 智能指针
│   ├── raii.md                # RAII 惯用法
│   ├── move_semantics.md      # 移动语义
│   └── lambda.md              # Lambda 表达式
│
├── 02_CS106L_Notes/           # 斯坦福 CS106L 笔记
│   ├── README.md              # 课程介绍
│   └── key_concepts.md        # 核心概念总结
│
├── 03_d2learn_Guide/          # d2learn.org 学习指南
│   ├── README.md              # 资源介绍
│   └── study_path.md          # 学习路径
│
└── practice/                  # 实战练习
    ├── README.md              # 练习说明
    ├── cpp_led_rewrite/       # C++ 重写 LED 项目
    └── cpp_gpio_class/        # GPIO 类封装 (RAII 示例)
```

---

## 🎯 学习目标

完成本模块后，你将能够：

| 能力维度 | 具体目标 | 验收标准 |
|---------|---------|---------|
| **概念理解** | 解释 RAII、智能指针、移动语义 | 能口头清晰阐述概念 |
| **代码能力** | 手写实现 RAII 类、使用 3 种智能指针 | 代码通过编译无警告 |
| **项目实战** | 用 C++ 重写 C 项目 | 完成至少 2 个重写项目 |
| **内存安全** | 无内存泄漏、无悬空指针 | 通过 valgrind/ASan 检查 |

---

## 📚 前置要求

- ✅ 已完成 `01_toolchainS` 模块（GCC/Makefile 基础）
- ✅ 已完成 `02_BasicProgram` 模块（C 语言基础）
- ✅ 了解基本的面向对象概念

---

## 🔧 环境要求

```bash
# 编译器要求
g++ --version  # 需要 GCC 9+ 或 Clang 10+
# 或
clang++ --version

# 编译标志
-std=c++17 -Wall -Wextra -Wpedantic

# 内存检查工具（可选但推荐）
valgrind --version
# 或启用 AddressSanitizer
-fsanitize=address
```

---

## 📖 推荐学习顺序

### 第一阶段：基础语法（第 1-2 天）

1. 阅读 `01_Cpp17_Features/auto_type.md`
2. 完成其中的练习题
3. 阅读 `01_Cpp17_Features/lambda.md` 中的范围 for 部分

### 第二阶段：RAII 与智能指针（第 3-5 天）

1. 阅读 `01_Cpp17_Features/raii.md`
2. 阅读 `01_Cpp17_Features/smart_pointers.md`
3. 完成 `practice/cpp_gpio_class/` 练习

### 第三阶段：移动语义（第 6-8 天）

1. 阅读 `01_Cpp17_Features/move_semantics.md`
2. 理解左值/右值区别
3. 完成移动构造函数练习

### 第四阶段：项目实战（第 9-14 天）

1. 阅读 `02_CS106L_Notes/key_concepts.md`
2. 参考 `03_d2learn_Guide/study_path.md`
3. 完成 `practice/cpp_led_rewrite/` 项目

---

## 📝 验收检查清单

```markdown
## 概念检查
- [ ] 能解释 RAII 是什么，为什么重要
- [ ] 能说明 unique_ptr 和 shared_ptr 的区别
- [ ] 能解释 std::move 的作用
- [ ] 能说明 Lambda 捕获列表的用法

## 代码检查
- [ ] 手写实现一个 RAII 文件句柄类
- [ ] 正确使用 unique_ptr 管理动态数组
- [ ] 使用 shared_ptr 实现对象共享所有权
- [ ] 使用 Lambda 作为回调函数

## 项目检查
- [ ] 完成 LED 控制的 C++ 重写
- [ ] 完成 GPIO 类的 RAII 封装
- [ ] 代码通过编译无警告
- [ ] 通过内存泄漏检查
```

---

## 🔗 外部资源

| 资源 | 链接 | 用途 |
|-----|------|-----|
| Stanford CS106L | [链接](02_CS106L_Notes/README.md) | 现代 C++ 标准库 |
| d2learn.org | [链接](03_d2learn_Guide/README.md) | C++ 学习路径 |
| cppreference | https://en.cppreference.com/ | C++ 参考文档 |
| Compiler Explorer | https://godbolt.org/ | 在线编译测试 |

---

## 💡 学习建议

> **麦肯锡方法论提示**：采用"假设驱动"学习方式
> 
> 1. **先提出假设**：这个特性解决了什么问题？
> 2. **验证假设**：通过代码示例理解
> 3. **应用验证**：在实战练习中使用
> 4. **总结归纳**：形成自己的知识框架

---

## 📞 问题反馈

如遇到问题，请：
1. 首先检查代码是否能通过 [Compiler Explorer](https://godbolt.org/) 编译
2. 查阅 `cppreference.com` 确认语法
3. 在项目 issue 中提交问题

---

*最后更新：2026 年 3 月*
