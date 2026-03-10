/* led.c - C 风格 LED 控制实现 */
#include "led.h"
#include <string.h>

/* 全局状态数组 */
uint8_t led_states[NUM_LEDS] = {0};

/* 假设的硬件寄存器 */
#define GPIO_ODR  (*(volatile uint32_t*)0x48000014)

void led_init(void) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        led_states[i] = 0;
    }
    GPIO_ODR = 0;  /* 关闭所有 LED */
}

void led_set(int index, bool state) {
    if (index >= 0 && index < NUM_LEDS) {
        led_states[index] = state ? 1 : 0;
        /* 更新硬件 */
        if (state) {
            GPIO_ODR |= (1 << index);
        } else {
            GPIO_ODR &= ~(1 << index);
        }
    }
}

bool led_get(int index) {
    if (index >= 0 && index < NUM_LEDS) {
        return led_states[index] != 0;
    }
    return false;
}

void led_toggle(int index) {
    if (index >= 0 && index < NUM_LEDS) {
        led_set(index, !led_get(index));
    }
}

void led_all_off(void) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        led_states[i] = 0;
    }
    GPIO_ODR = 0;
}

void led_all_on(void) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        led_states[i] = 1;
    }
    GPIO_ODR = 0xFF;
}

int led_count_on(void) {
    int count = 0;
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        if (led_states[i]) {
            count++;
        }
    }
    return count;
}

int led_any_on(void) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        if (led_states[i]) {
            return 1;
        }
    }
    return 0;
}

void led_set_multiple(uint8_t mask) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        led_set(i, (mask & (1 << i)) != 0);
    }
}
