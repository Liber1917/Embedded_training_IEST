/* led.h - C 风格 LED 控制 */
#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_LEDS 8

/* LED 状态数组 */
extern uint8_t led_states[NUM_LEDS];

/* 初始化 LED */
void led_init(void);

/* 设置 LED 状态 */
void led_set(int index, bool state);

/* 获取 LED 状态 */
bool led_get(int index);

/* 切换 LED 状态 */
void led_toggle(int index);

/* 全部关闭 */
void led_all_off(void);

/* 全部开启 */
void led_all_on(void);

/* 点亮点亮的 LED 数量 */
int led_count_on(void);

/* 是否有 LED 点亮 */
int led_any_on(void);

/* 设置多个 LED 状态 */
void led_set_multiple(uint8_t mask);

#endif /* LED_H */
