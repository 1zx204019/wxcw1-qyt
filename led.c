#include "STC32G.h"
#include "config.h"
#include "led.h"
#include "relay.h"
#include <intrins.h>

unsigned char i;
// 点亮指定LED
void led_on(unsigned char n) {
    switch(n) {
        //case 1: s595_state |= (1 << 7); break; 
        case 1: 
            // 增加PWM占空比补偿亮度
            for(i=0; i<7; i++){
                s595_state |= (1 << 7);
                shift595_write(s595_state);
                delay_us(10);
            }
            break; 
        case 2: s595_state |= (1 << 6); break;
        case 3: s595_state |= (1 << 5); break;
        case 4: s595_state |= (1 << 4); break;
        case 5: s595_state |= (1 << 3); break;
        default: break;
    }
    shift595_write(s595_state);
}

// 关闭指定LED
void led_off(unsigned char n) {
    switch(n) {
        case 1: s595_state &= ~(1 << 7); break;
        case 2: s595_state &= ~(1 << 6); break;
        case 3: s595_state &= ~(1 << 5); break;
        case 4: s595_state &= ~(1 << 4); break;
        case 5: s595_state &= ~(1 << 3); break;
        default: break;
    }
    shift595_write(s595_state);
}
// 全部LED点亮
void led_all_on(void) {
    s595_state |= 0xF8; // 二进制11111000 (Q7-Q3)
    shift595_write(s595_state);
}
// 全部LED关闭
void led_all_off(void) {
    s595_state &= ~0xF8; // 同时清零Q3-Q7
    shift595_write(s595_state);
}

// 翻转指定LED状态
void led_toggle(unsigned char n) {
    switch(n) {
        //case 1: s595_state ^= (1 << 7); break;
        case 1: 
            // 增加PWM占空比补偿亮度
            for(i=0; i<7; i++){
                s595_state ^= (1 << 7);
                shift595_write(s595_state);
                delay_us(10);
            }
            break; 
        case 2: s595_state ^= (1 << 6); break;
        case 3: s595_state ^= (1 << 5); break;
        case 4: s595_state ^= (1 << 4); break;
        case 5: s595_state ^= (1 << 3); break;
        default: break;
    }
    shift595_write(s595_state);
}