#ifndef __LCD_H__
#define __LCD_H__
#include "STC32G.H"
#include "config.h"

// ========================= ST7567指令集 =========================
#define LCD_SOFT_RESET      0xE2    // 软复位
#define LCD_POWER_CTRL_1    0x2C    // 升压步骤1
#define LCD_POWER_CTRL_2    0x2E    // 升压步骤2
#define LCD_POWER_CTRL_3    0x2F    // 升压步骤3
#define LCD_CONTRAST_1      0x26    // 粗调对比度 (0x20-0x27)
#define LCD_CONTRAST_2      0x81    // 微调对比度设置
#define LCD_CONTRAST_VAL    0x3A    // 微调对比度值 (0x00-0x3F)
#define LCD_BIAS            0xA2    // 1/9偏压比
#define LCD_SEG_NORMAL      0xA0    // 列扫描顺序：从左到右
#define LCD_SEG_REVERSE     0xA1    // 列扫描顺序：从右到左
#define LCD_COM_NORMAL      0xC8    // 行扫描顺序：从上到下
#define LCD_COM_REVERSE     0xC0    // 行扫描顺序：从下到上
#define LCD_START_LINE      0x40    // 起始行设置 (0x40-0x7F)
#define LCD_DISPLAY_ON      0xAF    // 开显示
#define LCD_DISPLAY_OFF     0xAE    // 关显示
#define LCD_PAGE_ADDR       0xB0    // 设置页地址 (0xB0-0xB7)
#define LCD_COL_ADDR_H      0x10    // 设置列地址高4位 (0x10-0x1F)
#define LCD_COL_ADDR_L      0x00    // 设置列地址低4位 (0x00-0x0F)

// ========================= 尺寸宏定义 =========================
#define LCD_HEIGHT          64      // LCD高度：64像素
#define LCD_WIDTH           128     // LCD宽度：128像素
#define LCD_PAGES           8       // 页数：64/8=8页
#define LCD_MAX_ROW         (LCD_HEIGHT - 1)  // 最大行号(0-63)
#define LCD_MAX_COLUMN      (LCD_WIDTH - 8)   // 最大列号(考虑8列字符宽度)


extern unsigned char zeroFont[16];
extern unsigned char numstr[];

// ========================= 函数声明 =========================
// 基础功能函数
void LCD_Delay(unsigned int count);            // 毫秒级延时函数
void delay_us(unsigned int us);                // 微秒级延时函数（需在relay.c或lcd.c中实现）
void LCD_WriteByte(unsigned char dat);         // SPI发送一个字节
void LCD_WriteCommand(unsigned char command);  // 写命令到LCD
void LCD_WriteData(unsigned char dat);         // 写数据到LCD
void LCD_SetAddress(unsigned char page, unsigned char column); // 设置显示地址（页+列）
void LCD_Init(void);                           // LCD初始化
void LCD_Clear(void);                          // 清屏（清除所有页）
void LCD_ClearPages(unsigned char start_page, unsigned char end_page); // 清除指定页范围

// 背光控制函数
void LCD_BacklightOn(void);                    // 打开背光
void LCD_BacklightOff(void);                   // 关闭背光

// 字符/字符串显示函数
unsigned char GetCharIndex(unsigned char ch);  // 获取字符在字模表中的索引
unsigned char ReverseByte(unsigned char byte); // 翻转字节位序
void LCD_DisplayChar_ASCII(unsigned char page, unsigned char offset, unsigned char column, unsigned char ch); // 支持页内偏移的ASCII字符显示
void LCD_DisplayChar(unsigned char page, unsigned char column, unsigned char ch); // 显示8x16 ASCII字符
void LCD_DisplayString(unsigned char page, unsigned char column, unsigned char *str); // 显示ASCII字符串

// 汉字/特殊字符显示函数
void LCD_DISPLAYCHAR_NEW(unsigned char page, unsigned char column, unsigned int ch, unsigned char type); // 显示汉字/自定义字模

// 测试函数
void LCD_TestAllChars(void);                   // 测试所有ASCII字符显示

// ========================= 新菜单系统需要的函数声明 =========================
// 添加这些函数声明，用于新的菜单系统
void DisplayPage2(void);                       // 显示一级菜单
void DisplayPage6(void);                       // 显示二级菜单
void DisplayPage7(void);
void DisplayPage10(void);
void DisplayPage11(void);
void DisplayDetailPage(unsigned char page);         // 显示详情页面
void RefreshDisplay(void);                     // 刷新显示（外部调用）
void LCD_DisplayNumber(unsigned char row, unsigned char col, 
                      unsigned long num, unsigned char digits);


#endif  // __LCD_H__

