#include "reg51.h"
#include "key.h"
#include "led.h"
#include "uart4.h"
#include "config.h"
#include "STC32G.h"
#include "relay.h"

unsigned char key1_pressed = 0;
unsigned char key2_pressed = 0;
unsigned char key3_pressed = 0;
unsigned char key4_pressed = 0;
bit key_state_changed = 0; // 状态变化标志

unsigned char Key_GetValue(void) {
    unsigned char key = 0;
    // 优先级：按键1 > 按键2 > 按键3 > 按键4（防止多键同时触发冲突）
    if (key1_pressed) {
        key = 1;
        key1_pressed = 0; // 清除标志
    } else if (key2_pressed) {
        key = 2;
        key2_pressed = 0;
    } else if (key3_pressed) {
        key = 3;
        key3_pressed = 0;
    } else if (key4_pressed) {
        key = 4;
        key4_pressed = 0;
    }
    return key;
}

void key_scan(void) {
    static bit last_key1 = 1, last_key2 = 1, last_key3 = 1, last_key4 = 1;
    static unsigned int key1_delay = 0, key2_delay = 0, key3_delay = 0, key4_delay = 0;
    
    // KEY1 消抖处理 (10ms延时，假设1ms调用一次该函数)
    if (KEY1_PIN != last_key1) {
        key1_delay++;
        if (key1_delay >= 10) {  // 修正：10ms（原100偏长）
            if (KEY1_PIN == 0) {  // 按键按下（低电平有效）
                key1_pressed = 1;
                key_state_changed = 1;
            }
            last_key1 = KEY1_PIN;
            key1_delay = 0;
        }
    } else {
        key1_delay = 0;
    }
    
    // KEY2 消抖处理
    if (KEY2_PIN != last_key2) {
        key2_delay++;
        if (key2_delay >= 10) {
            if (KEY2_PIN == 0) {
                key2_pressed = 1;
                key_state_changed = 1;
            }
            last_key2 = KEY2_PIN;
            key2_delay = 0;
        }
    } else {
        key2_delay = 0;
    }
    
    // KEY3 消抖处理
    if (KEY3_PIN != last_key3) {
        key3_delay++;
        if (key3_delay >= 10) {
            if (KEY3_PIN == 0) {
                key3_pressed = 1;
                key_state_changed = 1;
            }
            last_key3 = KEY3_PIN;
            key3_delay = 0;
        }
    } else {
        key3_delay = 0;
    }
    
    // KEY4 消抖处理
    if (KEY4_PIN != last_key4) {
        key4_delay++;
        if (key4_delay >= 10) {
            if (KEY4_PIN == 0) {
                key4_pressed = 1;
                key_state_changed = 1;
            }
            last_key4 = KEY4_PIN;
            key4_delay = 0;
        }
    } else {
        key4_delay = 0;
    }
}

