# 工具链选择和配置

> 本模块帮助学习者选择和配置嵌入式开发工具链，从根本上理解C语言程序如何变成可执行文件并最终烧录到MCU中运行。

---

## 目录

1. [从想法到烧录：程序是如何运行的](#从想法到烧录程序是如何运行的)
2. [工具链概述](#工具链概述)
3. [Windows平台配置](#windows平台配置)
4. [常用IDE对比](#常用ide对比)
5. [第一个工程：Hello World](#第一个工程hello-world)
6. [进阶：MCU开发流程](#进阶mcu开发流程)
7. [常见问题排查](#常见问题排查)

---

## 从想法到烧录：程序是如何运行的

### C语言程序编译流程

```
源代码(c/a.c)  →  预处理器  →  编译器(cc1)  →  汇编器(as)  →  链接器(ld)  →  可执行文件(a.out)
     │              │              │              │             │
  .c/.h       展开宏、包含    生成汇编代码      生成机器码     合并目标文件
              头文件
```

### MCU烧录流程

```
可执行文件(.elf/.bin)  →  调试器(J-Link/ST-Link)  →  MCU Flash
       │                            │
  芯片格式                    SWD/JTAG协议
  (bin/hex/elf)              烧录到内部Flash
```

### 关键概念

| 概念 | 说明 |
|------|------|
| **交叉编译** | 在x86电脑上编译出ARM等目标架构可执行文件 |
| **目标文件** | .o文件，机器码但未链接 |
| **可执行文件** | .elf/.bin/.hex，链接后的完整程序 |
| **烧录** | 将程序写入MCU内部Flash存储器 |

---

## 工具链概述

### 嵌入式开发工具链组成

```
┌─────────────────────────────────────────────────────────┐
│                    完整工具链                            │
├─────────────────────────────────────────────────────────┤
│  编译器(Compiler)     gcc-arm-none-eabi / clang         │
│         ↓                                               │
│  汇编器(Assembler)    as                                 │
│         ↓                                               │
│  链接器(Linker)       ld / gcc                          │
│         ↓                                               │
│  调试器(Debugger)     openocd / J-Link GDB Server       │
│         ↓                                               │
│  烧录工具(Programmer) st-flash / pyocd / J-Link        │
│         ↓                                               │
│  库文件(Libraries)    CMSIS / HAL / LL                 │
└─────────────────────────────────────────────────────────┘
```

### 主流工具链

| 工具链 | 适用架构 | 特点 |
|--------|----------|------|
| gcc-arm-none-eabi | ARM Cortex-M/R | 开源免费、生态完善 |
| Arm Compiler 6 | ARM | 商业优化、IAR配套 |
| RISC-V GCC | RISC-V | 开源、逐渐流行 |
| LLVM/Clang | 多架构 | 现代编译器、速度快 |

---

## Windows平台配置

### 推荐开发环境：VSCode + GCC

#### 1. 安装VSCode

访问 https://code.visualstudio.com/ 下载安装

**推荐插件**:
- `C/C++` - IntelliSense代码补全
- `C/C++ Extension Pack` - 扩展包
- ` Cortex-Debug` - ARM调试
- `Embedded IDE` - 嵌入式开发
- `GitLens` - Git增强

#### 2. 安装GNU Arm Embedded Toolchain

```bash
# 下载地址: https://developer.arm.com/downloads/-/gnu-rm
# 选择 Windows ZIP 文件
# 解压到: C:\Program Files\ArmGNU\bin
```

验证安装:
```bash
arm-none-eabi-gcc --version
```

#### 3. 安装调试工具

**J-Link** (推荐):
```bash
# 下载: https://www.segger.com/downloads/j-link/
# 安装后包含 J-Link GDB Server
```

**ST-Link**:
```bash
# 安装驱动: ST-Link V2 USB驱动
# 工具: ST-LINK CLI 或 OpenOCD
```

#### 4. OpenOCD安装(可选)

```bash
# 下载: https://gnutoolchains.com/arm-eabi/
# 或使用预编译版本
```

---

## 常用IDE对比

| IDE | 优点 | 缺点 | 适合人群 |
|-----|------|------|----------|
| **Keil MDK** | 调试强大、生态完善 | 商业License费用高 | 入门/商业项目 |
| **IAR EWARM** | 优化好、可靠性高 | 商业License | 汽车/工业 |
| **Clion + EIDE** | 现代界面、开源免费 | 调试配置复杂 | 现代开发 |
| **VSCode + PlatformIO** | 插件丰富、跨平台 | 功能全但重 | 创客/爱好者 |
| **STM32CubeIDE** | 自动代码生成、免费 | 锁定ST芯片 | STM32开发 |

### 开发环境推荐

```
入门推荐:  Keil MDK (有License) 或 STM32CubeIDE (免费)

进阶推荐:  Clion + EIDE + GCC + OpenOCD

专业推荐:  VSCode + PlatformIO 或 CLion + 手动配置
```

---

## 第一个工程：Hello World

### 环境验证：xmake方式

#### 1. 安装xmake

```powershell
# Windows (管理员权限)
irm https://xmake.io/get.ps1 | iex

# 验证
xmake --version
```

#### 2. 创建C工程

```bash
# 创建工程
xmake create hello_world
cd hello_world

# 编写 main.c
# 查看输出
xmake -v
```

#### 3. 配置嵌入式工程(可选)

`xmake.lua` 配置示例:

```lua
-- xmake.lua
add_rules("mode.debug", "mode.release")

target("blink_led")
    set_kind("binary")
    add_files("src/*.c")
    set_toolchains("armgcc")
    add_defines("STM32F103x6", "USE_HAL_DRIVER")
    add_incdirs("include")
```

### 嵌入式Hello World: 点亮LED

> 这是嵌入式开发的第一步，类似传统编程的"Hello World"

**所需硬件**:
- STM32F103C8T6 开发板(或任意STM32)
- J-Link 或 ST-Link 调试器

**使用STM32CubeIDE快速创建**:

1. 打开 STM32CubeIDE → File → New → STM32 Project
2. 选择芯片型号: STM32F103C8Tx
3. 引脚配置: PC13(LED) 配置为 GPIO_Output
4. 生成代码
5. 在 `main.c` 的 `while(1)` 中添加:

```c
// 闪烁LED
HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
HAL_Delay(500);  // 500ms延迟
```

6. 编译 → 下载 → 观察LED闪烁

---

## 进阶：MCU开发流程

### 完整开发流程图

```
┌─────────────────────────────────────────────────────────┐
│                    MCU开发流程                           │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  1. 需求分析                                           │
│     └─ 功能定义、性能指标                               │
│           ↓                                            │
│  2. 硬件选型                                           │
│     └─ MCU选型(STM32/ESP32/...)                        │
│           ↓                                            │
│  3. 原理图设计                                         │
│     └─ 最小系统、外设电路                               │
│           ↓                                            │
│  4. PCB设计                                            │
│     └─ 制板、焊接                                       │
│           ↓                                            │
│  5. 软件开发                                           │
│     ├─ 驱动开发                                        │
│     ├─ 中间件                                          │
│     └─ 应用逻辑                                        │
│           ↓                                            │
│  6. 调试验证                                           │
│     ├─ 单元测试                                        │
│     ├─ 集成测试                                        │
│     └─ 系统测试                                        │
│           ↓                                            │
│  7. 量产烧录                                           │
│     └─ 量产工具、批量烧录                               │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 调试技巧

#### 常用调试手段

| 调试方法 | 适用场景 | 工具 |
|----------|----------|------|
| **printf** | 简单信息输出 | ITM/uart |
| **断点** | 代码逻辑调试 | IDE调试器 |
| **变量观察** | 运行时状态 | 调试器 |
| **寄存器查看** | 外设配置检查 | 调试器 |
| **示波器** | 硬件信号时序 | 示波器 |
| **逻辑分析仪** | 协议时序分析 | 逻辑分析仪 |

#### 常见问题排查

**问题1: 程序烧录后不运行**

```markdown
检查清单:
□ BOOT引脚配置正确
□ Flash内容被擦除
□ 时钟配置(HAL_RCC_OscConfig)
□ 启动文件(.s)是否正确链接
```

**问题2: 调试无法连接**

```markdown
检查清单:
□ 调试器驱动安装
□ SWDIO/SWCLK接线正确
□ 目标板供电正常
□ NRST引脚状态
```

---

## 相关资源

### 官方文档

- [ARM CMSIS文档](https://arm-software.github.io/CMSIS_5/)
- [ST HAL库文档](https://www.st.com/en/development-tools/stm32cubemx.html)
- [FreeRTOS文档](https://www.freertos.org/)

### 学习资源

- 正点原子《 STM32F4库函数开发指南》
- 野火《 FreeRTOS内核实现与应用开发》
- 《嵌入式系统设计》- 内核驱动方向

### 工具下载

- [ARM GCC Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
- [OpenOCD](https://openocd.org/)
- [ST-LINK Utility](https://www.st.com/en/development-tools/stsw-link004.html)
