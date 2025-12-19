#ifndef _RELAY_H_
#define _RELAY_H_

void relay_init(void);
void relay_on(unsigned char n);
void relay_off(unsigned char n);
void relay_all_off(void); // 添加此行声明
void shift595_write(unsigned char dat);
void delay_us(unsigned int us);
extern unsigned char s595_state; // 添加共享状态变量声明

#endif 