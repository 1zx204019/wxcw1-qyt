#include "config.h"
#include "relay.h"
#include "STC32G.h"
#include <intrins.h>
#include "uart4.h"

void delay_us(unsigned int us)
{
    unsigned int i;
    // 24MHz时钟下，12T模式，一个机器周期为0.5us
    // 这里的循环次数需要根据实际情况调整
    for(i = 0; i < us; i++)
    {
        _nop_();  // 执行一个空操作指令
        _nop_();
        _nop_();
        _nop_();
    }
}

unsigned char s595_state = 0x00; // 全局变量

void delay_ms(unsigned long ms)
{
    unsigned long i;
    for(i = 0; i < ms; i++)
    {
        delay_us(1000);  // 1ms = 1000微秒
    }
}

// 向74LS595写入8位数据
void shift595_write(unsigned char dat) {
    unsigned char i;
    SRCLK_PIN = 0;
    RCLK_PIN = 0;
    delay_us(1);
    for(i=0; i<8; i++) {
        SER_PIN = (dat & 0x80) ? 1 : 0;  // SER_PIN = P4.6
        // 延迟1us确保数据稳定
        delay_us(1);

        SRCLK_PIN = 1;  // SCLK_PIN = P4.5
        delay_us(20);
        SRCLK_PIN = 0;  // SCLK_PIN = P4.5
        delay_us(20);
        dat <<= 1;// 处理下一位
    }
    // 8位数据移位完成后，产生单个RCLK锁存脉冲
    RCLK_PIN = 1;  // RCLK_PIN = P2.7
    delay_us(30);
    RCLK_PIN = 0;  // RCLK_PIN = P2.7
    delay_us(10);
}

void relay_init(void) {
    SRCLK_PIN=0;
    RCLK_PIN=0;
    delay_ms(10);
    SER_PIN=0;
    s595_state = 0x08;
    shift595_write(s595_state);
    OE_PIN=1;
}
void relay_on(unsigned char n) {
    if(n == 1) s595_state |= (1 << 0); // Q0控制K1
    else if(n == 2) s595_state |= (1 << 1); // Q1控制K2
    shift595_write(s595_state);
}

void relay_off(unsigned char n) {
    if(n == 1) s595_state &= ~(1 << 0);
    else if(n == 2) s595_state &= ~(1 << 1);
    shift595_write(s595_state);
} 
// 添加以下新函数
void relay_all_off(void) {
    s595_state &= ~((1 << 0) | (1 << 1)); // 同时关闭Q0和Q1
    shift595_write(s595_state);
}

