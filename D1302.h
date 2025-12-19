#include "D1302.h"
#include "reg51.h"
#include "STC32G.h" 
#include "config.h"
#include <intrins.h>

// DS1302寄存器地址
#define DS1302_SEC   0x80
#define DS1302_MIN   0x82
#define DS1302_HOUR  0x84
#define DS1302_DATE  0x86
#define DS1302_MONTH 0x88
#define DS1302_DAY   0x8A
#define DS1302_YEAR  0x8C
#define DS1302_CTRL  0x8E

// 辅助函数：写入一个字节到DS1302
static void ds1302_write_byte(unsigned char cmd, unsigned char dat) {
    unsigned char i;
    DS1302_RST = 1;
    for(i=0; i<8; i++) {
        DS1302_SCLK = 0;
        DS1302_IO = cmd & 0x01;
        cmd >>= 1;
        DS1302_SCLK = 1;
    }
    for(i=0; i<8; i++) {
        DS1302_SCLK = 0;
        DS1302_IO = dat & 0x01;
        dat >>= 1;
        DS1302_SCLK = 1;
    }
    DS1302_RST = 0;
}

// 辅助函数：读取一个字节从DS1302
static unsigned char ds1302_read_byte(unsigned char cmd) {
    unsigned char i, dat = 0;
    cmd |= 0x01; // 读命令
    DS1302_RST = 1;
    for(i=0; i<8; i++) {
        DS1302_SCLK = 0;
        DS1302_IO = cmd & 0x01;
        cmd >>= 1;
        _nop_();_nop_(); // 添加2个NOP确保数据稳定
        DS1302_SCLK = 1;
        _nop_();_nop_();
    }
    for(i=0; i<8; i++) {
        DS1302_SCLK = 0;
        dat >>= 1;
        _nop_();_nop_();
        if(DS1302_IO) dat |= 0x80;
        DS1302_SCLK = 1;
        _nop_();_nop_();
    }
    DS1302_RST = 0;
    return dat;
}

void rtc_init(void) {
    // 关闭写保护
    ds1302_write_byte(DS1302_CTRL, 0x00);
}

void rtc_read(rtc_time_t *t) {
    t->sec  = ds1302_read_byte(DS1302_SEC);
    t->min  = ds1302_read_byte(DS1302_MIN);
    t->hour = ds1302_read_byte(DS1302_HOUR);
    t->day  = ds1302_read_byte(DS1302_DATE);
    t->mon  = ds1302_read_byte(DS1302_MONTH);
    t->year = ds1302_read_byte(DS1302_YEAR);
}

void rtc_write(rtc_time_t *t) {
    ds1302_write_byte(DS1302_CTRL, 0x00); // 关闭写保护
    ds1302_write_byte(DS1302_SEC, t->sec);
    ds1302_write_byte(DS1302_MIN, t->min);
    ds1302_write_byte(DS1302_HOUR, t->hour);
    ds1302_write_byte(DS1302_DATE, t->day);
    ds1302_write_byte(DS1302_MONTH, t->mon);
    ds1302_write_byte(DS1302_YEAR, t->year);
    ds1302_write_byte(DS1302_CTRL, 0x80); // 开启写保护
}

// 简单测试函数
void rtc_test(void) {
    rtc_time_t t;
    rtc_init();
    rtc_read(&t);
    // 可在此处添加串口打印时间等调试代码
}