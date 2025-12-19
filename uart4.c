#include "uart4.h"
#include <STC32G.H>
#include "lcd.h"
#include "config.h"
#include "relay.h"
#include "led.h"  // ??LED???

// ------------------- ???? -------------------
unsigned char uart_rx_buff[UART_BUFF_SIZE] = {0};  // ?????
unsigned char uart_rx_len = 0;                     // ??????
bit uart_rx_complete = 0;                          // ??????
unsigned long delay_count = 0;                     // ?????

// ????
DataPoint history[HISTORY_SIZE];
unsigned char history_index = 0;

// ????
Statistics current_stats;

// ------------------- ?????? -------------------
// ???????
MenuState menu_state;

// ???? (??config.h???,????)
//extern PageType current_page;  // ????
bit display_labels_initialized = 0;  // ??????????

// ------------------- ?????? -------------------
static void UART4_ClearBuffer(unsigned char *ptr, unsigned int len);
static void UART4_ParseData(void);
static void DisplayFixedLabels(void);
static void ClearDataArea(void);
static void ClearSummaryArea(void);
static void DisplayPage2(void);
static void DisplayDetailPage(PageType page);
static void HandlePrevItem(void);
static void HandleNextItem(void);
static void HandleEnterKey(void);
static void HandleReturnKey(void);
static void RefreshDisplay(void);
static void DisplayPage7(void);
static void DisplayPage12(void);
// ------------------- ??????? -------------------
void Menu_Init(void) {
    menu_state.current_page = PAGE_1;
	 menu_state.prev_page = PAGE_1; 
    menu_state.page2_selected = 0;
    menu_state.page7_selected = 0;  // ???PAGE_7????0
	  menu_state.page10_selected = 0; 
	 menu_state.page11_selected = 0; 
	menu_state.page12_selected = 0;
    menu_state.page_changed = 1;
    menu_state.menu_initialized = 0;
}

// ------------------- ??????? -------------------
void InitStatistics(void) {
    unsigned int i;
    
    // ??????
    for (i = 0; i < HISTORY_SIZE; i++) {
        history[i].pid = 0;
        history[i].aid = 0;
        history[i].temp = 0;
        history[i].volt1 = 0;
        history[i].volt2 = 0;
        history[i].fosc = 0;
    }
    history_index = 0;
    
    // ??????
    current_stats.total_slaves = 0;
    current_stats.total_data_count = 0;
    current_stats.overall_avg_temp = 0;
    current_stats.overall_max_temp = 0;
    current_stats.overall_min_temp = 255;
}

// ------------------- UART4??? -------------------
void UART4_Init(unsigned long baudrate)
{
    unsigned int reload = (unsigned int)(65536UL - (FOSC / (4UL * baudrate)));
    
    // IO????
    EAXFR = 1;
    P_SW2 |= 0x80;
    S4_S = 1;  // UART4??P5.2(RX)/P5.3(TX)
    P_SW2 &= ~0x80;
    
    // ??IO??:P5.2(RX)????,P5.3(TX)?????
    P5M1 |= (1 << 2);
    P5M0 &= ~(1 << 2);
    P5M1 &= ~(1 << 3);
    P5M0 |= (1 << 3);
    
    // ????
    S4CON = 0x50;  // 8???,?????,????
    T4L = reload & 0xFF;
    T4H = reload >> 8;
    T4T3M |= 0x80;  // ?????4
    T4T3M |= 0x20;  // ???4??UART4???????
    T4T3M &= ~0x10; // ???4?1T??
    
    // ????(??:???????)
    IE2 |= 0x10;    // ??UART4??
    AUXR |= 0x80;   // STC32G??:??????
    
    // ????????????
    InitStatistics();
    Menu_Init();
}

// ------------------- ?????? -------------------
void UART4_SendByte(unsigned char dat)
{
    S4BUF = dat;
    while (!(S4CON & 0x02));  // ??????
    S4CON &= ~0x02;           // ????????
}

void UART4_SendString(unsigned char *str)
{
    while (*str != '\0')
    {
        UART4_SendByte(*str);
        str++;
    }
}

void UART4_SendNumber(unsigned long num, unsigned char digits)
{
    unsigned char i;
    unsigned long divisor = 1;
    
    for (i = 1; i < digits; i++) divisor *= 10;
    for (i = 0; i < digits; i++)
    {
        UART4_SendByte((unsigned char)((num / divisor) % 10 + '0'));
        divisor /= 10;
    }
}

// ------------------- ???0???(????) -------------------
void Timer0_Init(void)
{
    TMOD &= 0xF0;  // ?????0??
    TMOD |= 0x01;  // ???0??1(16????)
    TL0 = 0x30;    // 24MHz 1T???1ms??
    TH0 = 0xF8;
    TF0 = 0;       // ??????
    TR0 = 1;       // ?????0
    ET0 = 1;       // ?????0??
    EA = 1;        // ?????
}

// ???0??????(????)
void Timer0_ISR(void) interrupt 1
{
    TL0 = 0x30;
    TH0 = 0xF8;
    if (delay_count > 0) delay_count--;
}

// ------------------- ????? -------------------
static void UART4_ClearBuffer(unsigned char *ptr, unsigned int len)
{
    while(len--) *ptr++ = 0;
}

// ------------------- ??????(????) -------------------
static void ClearDataArea(void)
{
    switch (menu_state.current_page) {
        case PAGE_1:
            break;
        case PAGE_2:
            break;
        case PAGE_3:
            ClearSummaryArea();
            break;
    }
}

// ------------------- ?????? -------------------
static void ClearSummaryArea(void)
{
    // ??????
}

// ------------------- ?????? -------------------
static void DisplayFixedLabels(void)
{
    if (display_labels_initialized == 0)
    {
        LCD_Clear();  // ??
        
        switch (menu_state.current_page) {
            case PAGE_1:
                // ??1: ???(????)
                // ?1?(?0):??
                LCD_DISPLAYCHAR_NEW(0, 0, 0, 0);   // "?"
                LCD_DISPLAYCHAR_NEW(0, 8, 1, 0);   // "?"
                
                // "??(°C)" - ???48
                LCD_DISPLAYCHAR_NEW(0, 48, 0, 1);   // "?"
                LCD_DISPLAYCHAR_NEW(0, 56, 1, 1);   // "?"
                LCD_DISPLAYCHAR_NEW(0, 64, 0, 6);   // "("
                LCD_DISPLAYCHAR_NEW(0, 72, 0, 2);   // "°"
                LCD_DISPLAYCHAR_NEW(0, 80, 0, 7);   // "C"
                
                // "??(mV)" - ???88
                LCD_DISPLAYCHAR_NEW(0, 88, 0, 3);   // "?"
                LCD_DISPLAYCHAR_NEW(0, 96, 1, 3);   // "?"
                LCD_DISPLAYCHAR_NEW(0, 104, 0, 6);  // "("
                LCD_DisplayChar(0, 112, 'V');      // "V"
                LCD_DISPLAYCHAR_NEW(0, 120, 0, 7);  // ")"
                
                // ?7?(?6):??
                // "???" - ?0
                LCD_DISPLAYCHAR_NEW(6, 0, 0, 4);   // "?"
                LCD_DISPLAYCHAR_NEW(6, 8, 1, 4);   // "?"
                LCD_DISPLAYCHAR_NEW(6, 16, 2, 4);  // "?"
                
                // "???" - ?48
                LCD_DISPLAYCHAR_NEW(6, 48, 3, 4);  // "?"
                LCD_DISPLAYCHAR_NEW(6, 56, 1, 4);  // "?"
                LCD_DISPLAYCHAR_NEW(6, 64, 2, 4);  // "?"
                
                // "??" - ?112
                LCD_DISPLAYCHAR_NEW(6, 104, 0, 5);  // "?"
                LCD_DISPLAYCHAR_NEW(6, 112, 1, 5);  // "?"
                break;
                
            case PAGE_2:
                DisplayPage2();  // ??PAGE_2????
                break;
                
            case PAGE_3:
            case PAGE_4:
            case PAGE_5:
            case PAGE_7:
            case PAGE_8:
                DisplayDetailPage(menu_state.current_page);  // ??????
                break;
            case PAGE_9:
                DisplayDetailPage(menu_state.current_page);  // ??????
						case PAGE_10:  // ??:?????????
                DisplayDetailPage(menu_state.current_page);  // ?????
                break;
            case PAGE_11:
                DisplayPage11(); // 显示page11内容
                break;
						case PAGE_12:
                DisplayPage12();  // 显示PAGE_12内容
                break;

        }
        display_labels_initialized = 1;
    }
}

// ------------------- ??PAGE_2(????) -------------------
static void DisplayPage2(void)
{
    unsigned char start_index = 0;  // ??????
    unsigned char i;
    
    // ??????,??????
    // ????????2?,??????
    if (menu_state.page2_selected >= 3) {
        start_index = menu_state.page2_selected - 2;  // ???????????
        if (start_index > (PAGE2_ITEM_COUNT - 3)) {
            start_index = PAGE2_ITEM_COUNT - 3;
        }
    }
    
    // ???3???(??LCD????4?,??????)
    for (i = 0; i < 3 && (start_index + i) < PAGE2_ITEM_COUNT; i++) {
        unsigned char item_index = start_index + i;
        unsigned char row = i * 2;  // ???2?(?0??2??4)
        
        // ????
        if (item_index == menu_state.page2_selected) {
           LCD_DISPLAYCHAR_NEW(row, 0, 0, 25); 
        } else {
            LCD_DisplayChar(row, 0, ' ');
        }
        
        // ????????????
        switch(item_index) {
            case MENU_MEASURE_DATA:
                // ??"????"
                LCD_DISPLAYCHAR_NEW(row, 8, 0, 8);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 8);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 2, 8);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 3, 8);  // "?"
                break;
                
            case MENU_SENSOR_STATUS:
                // ??"?????"
                LCD_DISPLAYCHAR_NEW(row, 8, 0, 9);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 9);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 2, 9);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 3, 9);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 40, 4, 9);  // "?"
                break;
                
            case MENU_PARAM_QUERY:
                // ??"????"
                LCD_DISPLAYCHAR_NEW(row, 8, 0, 10);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 10);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 2, 10);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 3, 10);  // "?"
                break;
                
            case MENU_HISTORY_RECORD:
                // ??"????" - ??????
                LCD_DISPLAYCHAR_NEW(row, 8, 0, 13);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 13);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 2, 13);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 3, 13);  // "?"
                break;
                
            case MENU_DEVICE_MAINT:
                // ??"????" - ??????
                LCD_DISPLAYCHAR_NEW(row, 8, 0, 14);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 14);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 2, 14);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 3, 14);  // "?"
                break;
        }
    }
    
    // ?6?(?6):????
    // ???
    LCD_DISPLAYCHAR_NEW(6, 0, 0, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 8, 1, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 16, 2, 4);  // "?"
    
    // ???
    LCD_DISPLAYCHAR_NEW(6, 40, 3, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 48, 1, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 56, 2, 4);  // "?"
    
    // ??
    LCD_DISPLAYCHAR_NEW(6, 80, 0, 12);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 88, 1, 12);  // "?"
    
    // ??
    LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);  // "?"
}
// ------------------- ??PAGE_7(??????) -------------------
// ------------------- ??PAGE_7(??????) -------------------
static void DisplayPage7(void)
{
    unsigned char start_index = 0;  // ??????
    unsigned char i;
    unsigned char col;  // ?????col??
    
    // ??????3?,??????
    // LCD????3?(?0?2?4?),?????
    if (menu_state.page7_selected >= 3) {
        start_index = menu_state.page7_selected - 2;  // ?????????
        if (start_index > (PAGE7_ITEM_COUNT - 3)) {
            start_index = PAGE7_ITEM_COUNT - 3;
        }
    }
    
  for (i = 0; i < 3; i++) {
        unsigned char row = i * 2;
        // ???0?(????)
        LCD_DisplayChar(row, 0, ' ');
        // ???8-48??????
        for (col = 8; col < 56; col += 8) {
            LCD_DisplayChar(row, col, ' ');
        }
    }
    
    // ??3???
    for (i = 0; i < 3 && (start_index + i) < PAGE7_ITEM_COUNT; i++) {
        unsigned char item_index = start_index + i;
        unsigned char row = i * 2;
        
        // ?????(?????????)
        if (item_index == menu_state.page7_selected) {
           LCD_DISPLAYCHAR_NEW(row, 0, 0, 25);  // ??????
        } else {
            LCD_DisplayChar(row, 0, ' ');  // ???????
        }
        // ?????????????
        switch(item_index) {
            case MENU_SENSOR_EVENT_1:
                // ??"?????1"
						    LCD_DISPLAYCHAR_NEW(row, 8, 0, 1);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 1);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 0, 19);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 1, 19);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 40, 2, 19);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 48, 3, 19);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 56, 4, 19);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 64, 5, 19);  // "1"
                break;
                
            case MENU_SENSOR_EVENT_2:
                // ??"?????2"
                               LCD_DISPLAYCHAR_NEW(row, 8, 0, 1);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 1);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 0, 20);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 1, 20);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 40, 2, 20);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 48, 3, 20);  // "2"
						    LCD_DISPLAYCHAR_NEW(row, 56, 4, 20);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 64, 5, 20);  // "2"
                break;
                
            case MENU_SENSOR_EVENT_3:
                // ??"?????3"
       LCD_DISPLAYCHAR_NEW(row, 8, 0, 21);
				LCD_DISPLAYCHAR_NEW(row, 16, 1, 21);
        LCD_DISPLAYCHAR_NEW(row, 24, 2, 21);
				LCD_DISPLAYCHAR_NEW(row, 32, 3, 21);
				LCD_DISPLAYCHAR_NEW(row, 40, 4, 21);
				LCD_DISPLAYCHAR_NEW(row, 48, 5, 21);
				LCD_DISPLAYCHAR_NEW(row, 56, 6, 21);
				LCD_DISPLAYCHAR_NEW(row, 64, 7, 21);
				LCD_DISPLAYCHAR_NEW(row, 72, 8, 21);

                break;
                
            case MENU_HISTORY_DATA_QUERY:
                // ??"??????"
                             LCD_DISPLAYCHAR_NEW(row, 8, 0, 23);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 23);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 24, 2, 23);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 3, 23);  // "1"
						    LCD_DISPLAYCHAR_NEW(row, 40, 4, 23);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 48, 5, 23);  // "1"

                break;
                
            case MENU_MAX_TEMP_QUERY:
                // ??"??????"
                   LCD_DISPLAYCHAR_NEW(row, 8, 0, 24);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 16, 1, 24);  // "?"
						    LCD_DISPLAYCHAR_NEW(row, 24, 0, 23);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 32, 1, 23);  // "?"
						    LCD_DISPLAYCHAR_NEW(row, 40, 0, 1);   // "?"
                LCD_DISPLAYCHAR_NEW(row, 48, 1, 1);  // "?"
						    LCD_DISPLAYCHAR_NEW(row, 56, 4, 23);  // "?"
                LCD_DISPLAYCHAR_NEW(row, 64, 5, 23);  // "1"

                break;
        }
    }
    
    // ?6?:??????
    // "??"
    LCD_DISPLAYCHAR_NEW(6, 0, 0, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 8, 1, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 16, 2, 4);  // "?"
    
    // ???
    LCD_DISPLAYCHAR_NEW(6, 40, 3, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 48, 1, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 56, 2, 4);  // "?"
    
    // ??
    LCD_DISPLAYCHAR_NEW(6, 80, 0, 22);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 88, 1, 22);  // "?"
    
    // ??
    LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);  // "?"

}


// ------------------- ????????????? -------------------
// ------------------- ????????????? -------------------
static void DisplayPage10(void)
{
    unsigned char i;
    unsigned char row;
    
    // ??
    LCD_Clear();
    
    // ??3?????
    for (i = 0; i < 3; i++) {
        row = i * 2;  // ?0?2?4?
        
        // ????(???PAGE_2?????)
        if (i == menu_state.page10_selected) {
            LCD_DISPLAYCHAR_NEW(row, 0, 0, 25);  // ?PAGE_2?????
        } else {
            LCD_DisplayChar(row, 0, ' ');
        }
        
        // ??"??"???
        LCD_DISPLAYCHAR_NEW(row, 8, 2, 19);   // "?"
        LCD_DISPLAYCHAR_NEW(row, 16, 3, 19);  // "?"
        LCD_DisplayChar(row, 24, '1' + i);    // ??
    }
    
    // ?6?:????
		LCD_DISPLAYCHAR_NEW(0, 40, 2, 20);   // "?"
    LCD_DISPLAYCHAR_NEW(0, 48, 3, 20);   // "?"
    LCD_DISPLAYCHAR_NEW(0, 56, 0, 15);  // "?"
		LCD_DISPLAYCHAR_NEW(2, 40, 2, 20);   // "?"
    LCD_DISPLAYCHAR_NEW(2, 48, 3, 20);   // "?"
    LCD_DISPLAYCHAR_NEW(2, 56, 0, 15);  // "?"
		LCD_DISPLAYCHAR_NEW(4, 40, 2, 20);   // "?"
    LCD_DISPLAYCHAR_NEW(4, 48, 3, 20);   // "?"
    LCD_DISPLAYCHAR_NEW(4, 56, 0, 15);  // "?"
		
    LCD_DISPLAYCHAR_NEW(6, 0, 0, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 8, 1, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 16, 2, 4);  // "?"
    
    LCD_DISPLAYCHAR_NEW(6, 40, 3, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 48, 1, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 56, 2, 4);  // "?"
    
    LCD_DISPLAYCHAR_NEW(6, 72, 0, 26);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 80, 1, 26);  // "?"
		
    LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);  // "?"
}
// ------------------- ?????? -------------------

// 在uart4.c中添加page11显示函数
static void DisplayPage11() {
    LCD_Clear(); // 清屏

    // 第一行（行1，对应参数0）：ID: PID=    AID=
    // 显示"ID:"（ASCII字符串）
    LCD_DisplayString(0, 0, "ID:");
    // 显示"PID="（ASCII字符串）
    LCD_DisplayString(0, 24, "PID=");
    // 预留PID显示位置（4位数字），列地址从48开始
    // 显示"AID="（ASCII字符串），列地址从80开始
    LCD_DisplayString(0, 80, "AID=");

    // 第二行（行2，对应参数2）：温度：
    LCD_DISPLAYCHAR_NEW(2, 0, 0, 1);  // "温"（假设type=1对应"温"）
    LCD_DISPLAYCHAR_NEW(2, 8, 1, 1);  // "度"（同字模组）
    LCD_DisplayChar(2, 16, ':');      // 冒号

    // 第三行（行3，对应参数4）：电压：
    LCD_DISPLAYCHAR_NEW(4, 0, 0, 3);  // "电"（假设type=3对应"电"）
    LCD_DISPLAYCHAR_NEW(4, 8, 1, 3);  // "压"（同字模组）
    LCD_DisplayChar(4, 16, ':');      // 冒号

    // 第四行（行4，对应参数6）：删除、返回提示
    // 按1删除
    LCD_DISPLAYCHAR_NEW(6, 0, 0, 27);
    LCD_DISPLAYCHAR_NEW(6, 8, 1, 27);
    LCD_DISPLAYCHAR_NEW(6, 16, 2, 27);
    LCD_DISPLAYCHAR_NEW(6, 24, 3,27);
    // 按2返回

    LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);
    LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);

    // 显示当前选中的记录数据（从history数组读取）
    if (history_index > 0 && menu_state.page11_selected < history_index) {
        DataPoint *curr = &history[menu_state.page11_selected];
        // 显示PID（4位数字，列地址48）
        LCD_DisplayNumber(0, 48, curr->pid, 4);
        // 显示AID（4位数字，列地址104）
        LCD_DisplayNumber(0, 104, curr->aid, 4);
        // 显示温度值（列地址24）
        LCD_DisplayNumber(2, 24, curr->temp, 3);
        LCD_DISPLAYCHAR_NEW(2, 40, 0, 2);  // "°"（type=2）
        LCD_DisplayChar(2, 48, 'C');       // "C"（ASCII）
        // 显示电压值（列地址24）
        LCD_DisplayNumber(4, 24, curr->voltage, 3);  // 假设电压字段为voltage
        LCD_DisplayString(4, 40, "mV");    // 电压单位（ASCII）
    } else {
       
    }
}
static void DisplayPage12(void) {
    LCD_Clear(); // 清屏
    
    LCD_DISPLAYCHAR_NEW(2, 0, 0, 27);
    LCD_DISPLAYCHAR_NEW(2, 8, 1, 27);
    LCD_DISPLAYCHAR_NEW(2, 16, 2, 27);
    LCD_DISPLAYCHAR_NEW(2, 24, 3,27);
		    LCD_DISPLAYCHAR_NEW(6, 0, 0, 28);
    LCD_DISPLAYCHAR_NEW(6, 8, 1, 28);
	    LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);
    LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);
}                       
static void DisplayDetailPage(PageType page)
{
    LCD_Clear();
    
    switch(page) {
        case PAGE_3:  // ????????
            // ?0?:PID=1 TX0 ??:   ??:    mv
            LCD_DisplayString(0, 0, "PID");         // 0-31?
            LCD_DISPLAYCHAR_NEW(0, 24, 0, 16); 
            LCD_DisplayString(0, 72, "TX");          // 32-47?
            LCD_DISPLAYCHAR_NEW(0, 88, 0, 16); 
            
            LCD_DISPLAYCHAR_NEW(2, 0, 0, 1);         // "?" 48-55?
            LCD_DISPLAYCHAR_NEW(2, 8, 1, 1);         // "?" 56-63?
            LCD_DISPLAYCHAR_NEW(2, 16, 0, 15); 
            LCD_DISPLAYCHAR_NEW(2, 72, 0, 3);         // "?" 72-79?
            LCD_DISPLAYCHAR_NEW(2, 80, 1, 3);         // "?" 80-87?
            LCD_DISPLAYCHAR_NEW(2, 88, 0, 15); 
            
            LCD_DISPLAYCHAR_NEW(6, 0, 0, 4);         // "?" 0-7?
            LCD_DISPLAYCHAR_NEW(6, 8, 1, 4);         // "?" 8-15?
            LCD_DISPLAYCHAR_NEW(6, 16, 4, 4);        // "?" 16-23?
            
            // "???"
            LCD_DISPLAYCHAR_NEW(6, 60, 3, 4);        // "?" 40-47?
            LCD_DISPLAYCHAR_NEW(6, 68, 1, 4);        // "?" 48-55?
            LCD_DISPLAYCHAR_NEW(6, 76, 4, 4);        // "?" 56-63?
            
            // "??"
            LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);      // "?" 104-111?
            LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);      // "?" 112-119?
            break;
            
        case PAGE_4:  // ???????
            // ?0?:PID=1 TX0 ??:   ??:    mv
            LCD_DisplayString(0, 0, "PID");         // 0-31?
            LCD_DISPLAYCHAR_NEW(0, 24, 0, 16); 
            
            LCD_DisplayString(2, 0, "PID");         // 0-31?
            LCD_DISPLAYCHAR_NEW(2, 24, 0, 16); 
            
            LCD_DisplayString(4, 0, "PID");         // 0-31?
            LCD_DISPLAYCHAR_NEW(4, 24, 0, 16); 
            
            LCD_DISPLAYCHAR_NEW(6, 0, 0, 4);         // "?" 0-7?
            LCD_DISPLAYCHAR_NEW(6, 8, 1, 4);         // "?" 8-15?
            LCD_DISPLAYCHAR_NEW(6, 16, 4, 4);        // "?" 16-23?
            
            // "???"
            LCD_DISPLAYCHAR_NEW(6, 60, 3, 4);        // "?" 40-47?
            LCD_DISPLAYCHAR_NEW(6, 68, 1, 4);        // "?" 48-55?
            LCD_DISPLAYCHAR_NEW(6, 76, 4, 4);        // "?" 56-63?
            
            // "??"
            LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);      // "?" 104-111?
            LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);      // "?" 112-119?
            break;
            
        case PAGE_5:  // ??????
        
				LCD_DISPLAYCHAR_NEW(0, 0, 0, 17);
				LCD_DISPLAYCHAR_NEW(0, 32, 1, 17);
				
				LCD_DISPLAYCHAR_NEW(0, 48, 2, 17);
				LCD_DISPLAYCHAR_NEW(0, 72, 3, 17);
				LCD_DISPLAYCHAR_NEW(2, 0, 0, 17);
				LCD_DISPLAYCHAR_NEW(2, 32, 1, 17);
				
				LCD_DISPLAYCHAR_NEW(2, 48, 2, 17);
				LCD_DISPLAYCHAR_NEW(2, 72, 3, 17);
				LCD_DISPLAYCHAR_NEW(4, 0, 0, 17);
				LCD_DISPLAYCHAR_NEW(4, 32, 1, 17);
				LCD_DISPLAYCHAR_NEW(4, 48, 2, 17);
				LCD_DISPLAYCHAR_NEW(4, 72, 3, 17);
			
		LCD_DISPLAYCHAR_NEW(6, 0, 0, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 8, 1, 4);   // "?"
    LCD_DISPLAYCHAR_NEW(6, 16, 2, 4);  // "?"
    
    // ???
    LCD_DISPLAYCHAR_NEW(6, 40, 3, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 48, 1, 4);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 56, 2, 4);  // "?"
    
    // ??
    LCD_DISPLAYCHAR_NEW(6, 80, 0, 12);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 88, 1, 12);  // "?"
    
    // ??
    LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);  // "?"
    LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);  // "?"
				
            break;
            

            case PAGE_7:  // ???????
            DisplayPage7();  // ????????
            break; 
        case PAGE_8:  // ??????
            LCD_DisplayString(2, 48, "DEVICE MAINT");
            LCD_DisplayString(4, 60, "5");
				
            break;
				 case PAGE_9:  // ??????
					 
				 	LCD_DISPLAYCHAR_NEW(0, 0, 0, 17);
				LCD_DISPLAYCHAR_NEW(0, 32, 1, 17);
				 LCD_DISPLAYCHAR_NEW(2, 48, 2, 17);
				LCD_DISPLAYCHAR_NEW(2, 72, 3, 17);
            LCD_DISPLAYCHAR_NEW(6, 0, 0, 18);  // "?"
            LCD_DISPLAYCHAR_NEW(6, 8, 1, 18); 
				 
				   LCD_DISPLAYCHAR_NEW(6, 62, 0, 18);  // "?"
            LCD_DISPLAYCHAR_NEW(6, 70, 2, 18);
				 
				   LCD_DISPLAYCHAR_NEW(6, 112, 0, 11);  // "?"
            LCD_DISPLAYCHAR_NEW(6, 120, 1, 11);
				
            break;
      
        case PAGE_10:  // ??????????
            DisplayPage10();  // ??????????
            break;
        default:
            break;
    }
}

// ------------------- ???? -------------------
static void UART4_ParseData(void)
{
    if (uart_rx_len >= 6)
    {
        unsigned char pid_val = uart_rx_buff[0];
        unsigned char aid_val = uart_rx_buff[1];
        unsigned char temp_val = uart_rx_buff[2];
        unsigned char volt_val = uart_rx_buff[3];
        unsigned char volt2_val = uart_rx_buff[4];
        unsigned char fosc_val = uart_rx_buff[5];
        
        // ???????,??????
        AddDataToHistory(pid_val, aid_val, temp_val, volt_val, volt2_val, fosc_val);
    }
}

// ------------------- ????????? -------------------
void AddDataToHistory(unsigned char pid, unsigned char aid, unsigned char temp, 
                     unsigned char volt1, unsigned char volt2, unsigned char fosc)
{
    // ???0??(???????)
    if (pid == 0 && aid == 0 && temp == 0 && volt1 == 0 && volt2 == 0 && fosc == 0) {
        return;  // ?0??,??
    }
    
    // ????
    history[history_index].pid = pid;
    history[history_index].aid = aid;
    history[history_index].temp = temp;
    history[history_index].volt1 = volt1;
    history[history_index].volt2 = volt2;
    history[history_index].fosc = fosc;
    
    // ????
    history_index = history_index + 1;
    if (history_index >= HISTORY_SIZE) {
        history_index = 0;  // ?????
    }
    
    // ??????
    CalculateStatistics();
}

// ------------------- ?????? -------------------
void CalculateStatistics(void)
{
    unsigned int i;
    unsigned int j;
    unsigned int sum_temp;
    unsigned int total_count;
    unsigned char found;
    
    // ??AID
    unsigned char slave_ids[16];
    unsigned char slave_count;
    unsigned char aid_val;
    unsigned char temp_val;
    
    // ????
    for (i = 0; i < 16; i++) {
        slave_ids[i] = 0;
    }
    
    // ??????
    current_stats.total_slaves = 0;
    current_stats.total_data_count = 0;
    current_stats.overall_avg_temp = 0;
    current_stats.overall_max_temp = 0;
    current_stats.overall_min_temp = 255;
    
    sum_temp = 0;
    total_count = 0;
    slave_count = 0;
    
    // ??????
    for (i = 0; i < HISTORY_SIZE; i++) {
        temp_val = history[i].temp;
        
        // ??????:??0????
        if (temp_val > 0 && temp_val < 100) {
            aid_val = history[i].aid;
            
            // ????AID
            if (aid_val > 0) {
                found = 0;
                for (j = 0; j < slave_count; j++) {
                    if (slave_ids[j] == aid_val) {
                        found = 1;
                        break;
                    }
                }
                
                if (found == 0 && slave_count < 16) {
                    slave_ids[slave_count] = aid_val;
                    slave_count = slave_count + 1;
                }
            }
            
            // ????
            sum_temp = sum_temp + temp_val;
            
            if (temp_val > current_stats.overall_max_temp) {
                current_stats.overall_max_temp = temp_val;
            }
            if (temp_val < current_stats.overall_min_temp) {
                current_stats.overall_min_temp = temp_val;
            }
            
            total_count = total_count + 1;
        }
    }
    
    // ????
    current_stats.total_slaves = slave_count;
    
    if (total_count > 0) {
        current_stats.overall_avg_temp = (unsigned char)(sum_temp / total_count);
        current_stats.total_data_count = (unsigned char)total_count;
        
        // ???????????,???0
        if (current_stats.overall_min_temp == 255) {
            current_stats.overall_min_temp = 0;
        }
    } else {
        current_stats.overall_avg_temp = 0;
        current_stats.total_data_count = 0;
        current_stats.overall_min_temp = 0;
    }
}

// ------------------- ??????? -------------------
unsigned char CountDifferentSlaves(void)
{
    unsigned int i;
    unsigned int j;
    unsigned char slave_ids[16];
    unsigned char slave_count;
    unsigned char found;
    unsigned char aid_val;
    unsigned char temp_val;
    unsigned char k;
    
    // ??
    for (k = 0; k < 16; k++) {
        slave_ids[k] = 0;
    }
    slave_count = 0;
    
    // ????,????AID
    for (i = 0; i < HISTORY_SIZE; i++) {
        temp_val = history[i].temp;
        aid_val = history[i].aid;
        
        // ??????:?????????AID??0
        if (temp_val > 0 && temp_val < 100 && aid_val > 0) {
            // ???????
            found = 0;
            for (j = 0; j < slave_count; j++) {
                if (slave_ids[j] == aid_val) {
                    found = 1;
                    break;
                }
            }
            
            // ??????????,???
            if (found == 0 && slave_count < 16) {
                slave_ids[slave_count] = aid_val;
                slave_count = slave_count + 1;
            }
        }
    }
    
    return slave_count;
}

// ------------------- ????? -------------------
void ShowSlaveCount(void)
{
    unsigned char slave_count;
    unsigned char buf[10];
    PageType saved_page;
    
    // ?????
    slave_count = CountDifferentSlaves();
    
    // ??????
    saved_page = menu_state.current_page;
    
    // ??
    LCD_Clear();
    
    // ?1?:??(?2)
    LCD_DisplayString(0, 0, "SLAVE COUNT");
    LCD_DisplayString(2, 0, "DIFF AID:");
    
    // ????
    LCD_DisplayString(4, 0, "COUNT=");
    
    if (slave_count >= 100) {
        buf[0] = (slave_count / 100) + '0';
        buf[1] = ((slave_count % 100) / 10) + '0';
        buf[2] = (slave_count % 10) + '0';
        buf[3] = 0;
        LCD_DisplayString(4, 48, buf);
    } else if (slave_count >= 10) {
        buf[0] = (slave_count / 10) + '0';
        buf[1] = (slave_count % 10) + '0';
        buf[2] = 0;
        LCD_DisplayString(4, 48, buf);
    } else {
        buf[0] = slave_count + '0';
        buf[1] = 0;
        LCD_DisplayString(4, 48, buf);
    }
    
    LCD_DisplayString(6, 0, "AUTO RETURN...");
    
    // ??3?
    delay_ms(3000);
    
    // ????
    menu_state.current_page = saved_page;
    display_labels_initialized = 0;
    RefreshDisplay();
}


void UART4_ISR(void) interrupt 18
{
    if (S4CON & 0x01)  // ????
    {
        S4CON &= ~0x01; // ??????
        
        if (uart_rx_len < 6)  // ????(6??)
        {
            uart_rx_buff[uart_rx_len++] = S4BUF;
            if (uart_rx_len == 6)  // ??6??,??????
            {
                uart_rx_complete = 1;
            }
        }
        else  // ????,??
        {
            uart_rx_len = 0;
            uart_rx_complete = 0;
        }
    }
}

// ------------------- ???????(?????) -------------------
void UART4_ReceiveString(void)
{
    DisplayFixedLabels();  // ??????
    
    if (uart_rx_complete == 1)
    {
        UART4_ParseData();  // ????
        ClearDataArea();    // ??????
        
        // ????????????
        switch (menu_state.current_page) {
            case PAGE_1:
                // ?????????
                break;
                

                
            default:
                // ???????????
                break;
        }
        
        // ?????
        UART4_ClearBuffer(uart_rx_buff, 6);
        uart_rx_complete = 0;
        uart_rx_len = 0;
    }
}

// ------------------- ?????? -------------------
void ExportHistoryData(void)
{
    unsigned int i;
    
    UART4_SendString("\r\n=== HISTORY DATA ===\r\n");
    UART4_SendString("IDX PID AID TEM VOL1 VOL2 FOSC\r\n");
    
    for (i = 0; i < HISTORY_SIZE; i++) {
        if (history[i].temp > 0 || history[i].volt1 > 0) {
            // ??
            UART4_SendNumber(i, 3);
            UART4_SendString(" ");
            
            // ??PID
            UART4_SendNumber((unsigned long)history[i].pid, 3);
            UART4_SendString(" ");
            
            // ??AID
            UART4_SendNumber((unsigned long)history[i].aid, 3);
            UART4_SendString(" ");
            
            // ????
            UART4_SendNumber((unsigned long)history[i].temp, 3);
            UART4_SendString(" ");
            
            // ????1
            UART4_SendNumber((unsigned long)history[i].volt1, 3);
            UART4_SendString(" ");
            
            // ????2
            UART4_SendNumber((unsigned long)history[i].volt2, 3);
            UART4_SendString(" ");
            
            // ??FOSC
            UART4_SendNumber((unsigned long)history[i].fosc, 3);
            UART4_SendString("\r\n");
        }
    }
    
    // ????
    UART4_SendString("\r\n=== STATISTICS ===\r\n");
    UART4_SendString("COUNT:");
    UART4_SendNumber((unsigned long)current_stats.total_data_count, 3);
    UART4_SendString("\r\n");
    
    UART4_SendString("TEMP: AVG:");
    UART4_SendNumber((unsigned long)current_stats.overall_avg_temp, 3);
    UART4_SendString(" MAX:");
    UART4_SendNumber((unsigned long)current_stats.overall_max_temp, 3);
    UART4_SendString(" MIN:");
    UART4_SendNumber((unsigned long)current_stats.overall_min_temp, 3);
    UART4_SendString("\r\n");
}


void LCD_HandleKey(unsigned char key) {
    // 记录当前页面用于状态判断
	unsigned char i;
    PageType current_page = menu_state.current_page;

    switch(key) {
        case 1: // 按键1：PAGE_11进入PAGE_12；PAGE_12确认删除
            if (current_page == PAGE_11) {
                // 从PAGE_11进入PAGE_12
                menu_state.prev_page = PAGE_11; // 记录上一页
                menu_state.current_page = PAGE_12;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            } else if (current_page == PAGE_12) {
                // PAGE_12确认删除（执行删除逻辑）
                if (history_index > 0 && menu_state.page11_selected < history_index) {
                    // 覆盖删除选中的历史记录
                    for (i = menu_state.page11_selected; i < history_index - 1; i++) {
                        history[i] = history[i + 1];
                    }
                    history_index--; // 记录数减1
                    // 调整选中索引
                    if (menu_state.page11_selected >= history_index && history_index > 0) {
                        menu_state.page11_selected = history_index - 1;
                    }
                }
                // 删除后返回PAGE_11
                menu_state.current_page = PAGE_11;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            } else {
                HandlePrevItem(); // 其他页面保持原有逻辑
            }
            break;

        case 2: // 按键2：PAGE_12返回PAGE_11；PAGE_11返回PAGE_10；其他页下一项
            if (current_page == PAGE_12) {
                // 从PAGE_12返回PAGE_11
                menu_state.current_page = PAGE_11;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            } else if (current_page == PAGE_11) {
                // 从PAGE_11返回PAGE_10
                menu_state.current_page = PAGE_10;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            } else {
                HandleNextItem(); // 其他页面保持原有逻辑
            }
            break;
            
        case 3: // 按键3：进入/选择（扩展page11进入逻辑）
            if (current_page == PAGE_5) {
                // 从PAGE_5进入PAGE_9（原有逻辑）
                menu_state.current_page = PAGE_9;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            } else if (current_page == PAGE_9) {
                // 从PAGE_9返回PAGE_5（原有逻辑）
                menu_state.current_page = PAGE_5;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            } else if (current_page == PAGE_10) {
                // 从PAGE_10进入PAGE_11（新增逻辑）
                menu_state.prev_page = PAGE_10; // 记录上一级页面为PAGE_10
                menu_state.current_page = PAGE_11; // 切换到page11
                menu_state.page11_selected = 0; // 初始化选中第0条记录
                menu_state.page_changed = 1;
                display_labels_initialized = 0; // 强制刷新显示
            } else {
                // 其他页面保持原有"进入"功能
                HandleEnterKey();
            }
            break;
            
        case 4: // 按键4：返回（原有逻辑，page11不使用此键返回）
            if (current_page != PAGE_1) {
                HandleReturnKey();
            }
            break;
    }
    
    // 刷新显示
    RefreshDisplay();
}
// ------------------- ????? -------------------
static void HandlePrevItem(void)
{
    switch(menu_state.current_page) {
        case PAGE_2:
            if (menu_state.page2_selected > 0) {
                menu_state.page2_selected--;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;  // ??????
            }
            break;
          case PAGE_7:  // ??PAGE_7???
            if (menu_state.page7_selected > 0) {
                menu_state.page7_selected--;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            }
            break; 
          case PAGE_10:  // ??:PAGE_10???
            if (menu_state.page10_selected > 0) {
                menu_state.page10_selected--;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            }
            break;						
        default:
            break;
    }
}

// ------------------- ????? -------------------
static void HandleNextItem(void)
{
    switch(menu_state.current_page) {
        case PAGE_2:
            if (menu_state.page2_selected < (PAGE2_ITEM_COUNT - 1)) {
                menu_state.page2_selected++;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;  // ??????
            }
            break;
             case PAGE_7:  // ??PAGE_7???
            if (menu_state.page7_selected < (PAGE7_ITEM_COUNT - 1)) {
                menu_state.page7_selected++;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            }
            break;
						case PAGE_10:  // ??:PAGE_10???
            if (menu_state.page10_selected < 2) {  // ??3???(0,1,2)
                menu_state.page10_selected++;
                menu_state.page_changed = 1;
                display_labels_initialized = 0;
            }
            break;
        default:
            break;
    }
}

// ------------------- ????? -------------------
static void HandleEnterKey(void)
{
    switch(menu_state.current_page) {
        case PAGE_1:
            // ??????????
            menu_state.current_page = PAGE_2;
            menu_state.page2_selected = 0;  // ???????
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            led_toggle(3);
            break;
            
        case PAGE_2:
            // ???????????
            switch(menu_state.page2_selected) {
                case MENU_MEASURE_DATA:
                    menu_state.current_page = PAGE_3;
                    break;
                case MENU_SENSOR_STATUS:
                    menu_state.current_page = PAGE_4;
                    break;
                case MENU_PARAM_QUERY:
                    menu_state.current_page = PAGE_5;
                    break;
                case MENU_HISTORY_RECORD:
                    menu_state.current_page = PAGE_7;  // ????????
                    break;
                case MENU_DEVICE_MAINT:
                    menu_state.current_page = PAGE_8;  // ????????
                    break;
            }
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            break;
             if (menu_state.page7_selected == MENU_HISTORY_DATA_QUERY) {
                ShowSlaveCount();  // ?????????????
            } else {
      
                led_toggle(0);  // ??LED0????
            }
            break;
						
						
						 case PAGE_7:
            // ?????????
            switch(menu_state.page7_selected) {
                case MENU_SENSOR_EVENT_1:
                case MENU_SENSOR_EVENT_2:
                case MENU_SENSOR_EVENT_3:
                    // ???????????
                    menu_state.current_page = PAGE_10;
                    menu_state.page10_selected = menu_state.page7_selected;  // ??????
                    menu_state.page_changed = 1;
                    display_labels_initialized = 0;
                    break;
                    
                case MENU_HISTORY_DATA_QUERY:
                    ShowSlaveCount();  // ??????
                    break;
                    
                case MENU_MAX_TEMP_QUERY:
                    led_toggle(0);  // ??????
                    break;
            }
            break;
            
        case PAGE_10:
            // ?PAGE_10?,????????????
            // ??:????????????
            // ??????:??LED????
            led_toggle(1);
            break;
        default:
            break;
    }
}

// ------------------- ????? -------------------
static void HandleReturnKey(void)
{
    switch(menu_state.current_page) {
        case PAGE_2:
            // ?????
            menu_state.current_page = PAGE_1;
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            break;
            
        case PAGE_3:
        case PAGE_4:
        case PAGE_5:
          
            menu_state.current_page = PAGE_2;
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            break;
            
        case PAGE_7:

            menu_state.current_page = PAGE_2;
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            break;
            
        case PAGE_8:
            // ????????
            menu_state.current_page = PAGE_2;
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            break;
        case PAGE_9:  // ??????????
            menu_state.current_page = PAGE_2;  // ??????????PAGE_5
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            break;
				case PAGE_10:
            // ?????????????????
            menu_state.current_page = PAGE_7;
            menu_state.page_changed = 1;
            display_labels_initialized = 0;
            break;
        default:
            break;
    }
}

// ------------------- ???? -------------------
static void RefreshDisplay(void)
{
    if (menu_state.page_changed) {
        // ??DisplayFixedLabels?????
        display_labels_initialized = 0;
        DisplayFixedLabels();
        menu_state.page_changed = 0;
    }
}

