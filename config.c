#include "config.h"
#include "STC32G.h"

// GPIO初始化函数
void GPIO_Init(void) {
    // 按键(P0.0~P0.3) 高阻输入
    P0M0 &= ~0x0F;  // 清除推挽输出位
    P0M1 |= 0x0F;   // 设置高阻输入位

    // 595控制脚（P4.5, P2.7, P4.6,P0.4）推挽输出
    P4M0 |= (1<<5) | (1<<6);
    P4M1 &= ~((1<<5) | (1<<6));
    P2M0 |= (1<<7);
    P2M1 &= ~(1<<7);
    P0M0 |= (1<<4);
    P0M1 &= ~(1<<4);

    // 串口（P3.0, P3.1）
    P3M1 = 0x01;  // P3.0高阻输入(M1=1,M0=0)
    P3M0 = 0x02;  // P3.1推挽输出(M1=0,M0=1)
    P_SW1 &= ~0x30;  // 清除串口1选择位
    P_SW1 |= 0x00;   // 选择P3.0/P3.1为串口1
	
    // RTC DS1302（P2.6, P2.5, P2.4）
    P2M0 &= ~((1<<6) | (1<<5) | (1<<4));  // 清除推挽位
    P2M1 |= (1<<6) | (1<<5) | (1<<4);     // 设置开漏位
    
    //LCD
    // P2.1 (LCD_SDA)
    P2M0 |= (1 << 1);
    P2M1 &= ~(1 << 1);
    // P3.4 (LCD_SCL)
    P3M0 |= (1 << 4);
    P3M1 &= ~(1 << 4);
    // P5.0 (LCD_CS)
    P5M0 |= (1 << 0);
    P5M1 &= ~(1 << 0);
    // P4.0 (LCD_RES)
    P4M0 |= (1 << 0);
    P4M1 &= ~(1 << 0);
    // P3.5 (LCD_RS)
    P3M0 |= (1 << 5);
    P3M1 &= ~(1 << 5);
    // P2.3 (LCD_BLK)
    P2M0 |= (1 << 3);
    P2M1 &= ~(1 << 3);

    // RS485，无线测温模块配置引脚
//    P5M0 |= 0x18;  // P53 (RX2) 设为输入模式
//    P5M1 &= 0xE7;
//    
//    P5M0 |= 0x10;  // P52 (TX2) 设为推挽输出
//    P5M1 &= 0xDF;

    P5M1 |= (1 << 2);   // M1=1
    P5M0 &= ~(1 << 2);  // M0=0
    // P5.3 (TXD4)：推挽输出（发送引脚）
    P5M1 &= ~(1 << 3);  // M1=0
    P5M0 |= (1 << 3);   // M0=1
		
    
    P4M0 |= 0x01;  // P40 (RST) 设为推挽输出
    P4M1 &= 0xFE;
    
    P5M0 |= 0x20;  // P54 (CE) 设为推挽输出
    P5M1 &= 0xDF;
    
    P1M0 |= 0x02;  // P11 (TX1) 设为推挽输出
    P1M1 &= 0xFD;
    
    P1M0 |= 0x01;  // P10 (RX1) 设为输入模式
    P1M1 &= 0xFE;

    // 将其他未使用GPIO口配置为准双向模式
    P6M1 = 0x00; P6M0 = 0x00;  // P6口全部为准双向口
    P7M1 = 0x00; P7M0 = 0x00;  // P7口全部为准双向口
	// 特别注意：STC32G需要使能XSFR访问
    P_SW2 |= 0x80;  // 设置EAXFR=1，允许访问扩展SFR
    {
        unsigned int startup_delay;
        for(startup_delay = 0; startup_delay < 10000; startup_delay++);
    }
}

// 添加系统Tick计数器
static unsigned long system_tick = 0;

// 增加系统Tick
void SystemTick_Increment(void) {
    system_tick++;
}

// 获取当前系统Tick
unsigned long GetSystemTick(void) {
    return system_tick;
}

