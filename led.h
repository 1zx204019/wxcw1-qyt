#ifndef _LED_H_
#define _LED_H_

// 添加LED编号宏定义
#define LED1 1
#define LED2 2
#define LED3 3
#define LED4 4
#define LED5 5

void led_init(void);
void led_on(unsigned char n);
void led_off(unsigned char n);
void led_all_on(void);    // 新增：全部LED点亮
void led_all_off(void);   // 新增：全部LED关闭
void led_toggle(unsigned char n);  // 新增：LED状态翻转
extern unsigned char s595_state; // 共享595状态变量

#endif 
