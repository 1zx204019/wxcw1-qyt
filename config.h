#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "STC32G.h"
#include "led.h"  // 添加LED头文件

// 核心定义
#define FOSC 24000000UL  // 晶振频率24MHz
#define UART_BUFF_SIZE 6  // 串口缓冲区大小

// 原有的页面类型定义（移除，因为已在uart4.h中定义）
// #define PAGE_1 0
// #define PAGE_2 1
// #define PAGE_3 2
// typedef unsigned char PageType; // 移除此行

// 引脚定义保持不变...
#define LED1_POS   7
#define LED2_POS   6
#define LED3_POS   5
#define LED4_POS   4
#define LED5_POS   3
#define KEY1_PIN   P03
#define KEY2_PIN   P02
#define KEY3_PIN   P01
#define KEY4_PIN   P00
#define RELAY1_PIN 0
#define RELAY2_PIN 1
#define SRCLK_PIN  P45
#define RCLK_PIN   P27
#define SER_PIN    P46
#define OE_PIN     P04
#define UART_RX1   P30
#define UART_TX1   P31
#define DS1302_SCLK   P26
#define DS1302_IO     P25
#define DS1302_RST    P24
#define LCD_SCLK P35
#define LCD_SDA  P21
#define LCD_RST  P50
#define LCD_RS   P34
#define LCD_CS   P40
#define LCD_BL   P23

// 历史数据大小
#define HISTORY_SIZE 255

// 函数声明
void GPIO_Init(void);
unsigned long GetSystemTick(void);
void SystemTick_Increment(void);

#endif // _CONFIG_H_


