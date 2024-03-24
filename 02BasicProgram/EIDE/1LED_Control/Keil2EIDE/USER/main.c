#include "air32f10x.h"

// 一个工程里只能有一个main.c文件，一个main函数。
// 下面列举各种延时点灯方式以供参考
#include "air32f10x_rcc.h" // RCC（Reset and Clock Control）外设
#include "delay.h"
// 需要了解GPIO相关知识，板载灯分别对应PB11，PB10，PB2
#include "air32f10x_gpio.h"

// 延时点灯：宏定义寄存器版
// 这种方式更加简洁和省时，但是不易读
#define LED_OFF  GPIOB -> BRR = GPIO_Pin_11
#define LED_ON  GPIOB -> BSRR = GPIO_Pin_11
#define LED_TOGGLE GPIOB -> ODR ^= GPIO_Pin_11 // ODR为输出状态寄存器，异或即可翻转

// 延时点灯：函数版
int main()
{
	// 配置GPIO, 使能GPIOB时钟，两种方法都绕不开的步骤
	// 函数方法更易上手
	configure_GPIO();
	// 通过PB11控制板载灯
	GPIO_SetBits(GPIOB,GPIO_Pin_11); // 点亮PB11
	// 延时2s
	Delay_Init(); // 初始化延时函数
	Delay_S(2);

	GPIO_ResetBits(GPIOB,GPIO_Pin_11);

	// 下面把延时和点灯组合起来放入循环
	for(;;)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_11);
		// LED_ON;
		Delay_S(1);
		// GPIO_ResetBits(GPIOB,GPIO_Pin_11);
		LED_OFF;
		Delay_S(1);
	}

}

// 写一个配置GPIO的函数，免得挤在main中
// C系的函数位置没有讲究，放在main前面或后面都可以
void configure_GPIO(void)
{
	// 首先，声明一个GPIO_InitTypeDef结构体变量
	GPIO_InitTypeDef GPIO_InitStructure;

	// 使用GPIO_StructInit函数初始化结构体，设置默认值用于防止意外
    GPIO_StructInit(&GPIO_InitStructure);

	// 指定要配置的GPIO引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;

	// 指定GPIO引脚的速度
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 选择一个速度，例如50MHz，这个项目中随便选(参见air32f10x_gpio.h)

	// 指定GPIO引脚的模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 选择推挽输出，GPIO模式选择可参考https://durant35.github.io/2017/11/30/TACouses_ES2017_MCU_GPIO/
	
	// 现在，我们已经配置好了GPIO_InitStructure结构体
    // 接下来，我们需要使能GPIOB的时钟（如果是第一次使用GPIOB），先使能时钟才能使用GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// 应用配置
	GPIO_Init(GPIOB, &GPIO_InitStructure); // PB11，故第一个位置填GPIOB，后一个位置填上面的GPIO_InitStructure地址
}


