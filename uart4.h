#ifndef __UART4_H__
#define __UART4_H__

#include "STC32G.H"
#include "config.h"  // 包含配置文件（核心宏定义）
#include "led.h"     // LED相关头文件（保留原有依赖）

// -------------------------- 核心分区存储宏定义（优先使用新配置） --------------------------
// 总从站数/历史记录数配置（覆盖旧的HISTORY_SIZE）
#define TOTAL_SLAVES      35       // 总从站数(35个)
#define RECORDS_PER_SLAVE 4        // 每个从站历史记录数(4条)
#define RECENT_RECORDS    1        // 每个从站最近记录数(1条)
#define TOTAL_RECORDS     (TOTAL_SLAVES * (RECENT_RECORDS + RECORDS_PER_SLAVE))  // 总记录数=175

// 分区索引定义
#define RECENT_DATA_START 0                          // 最近数据起始索引
#define RECENT_DATA_SIZE  TOTAL_SLAVES               // 最近数据大小(35条)
#define HISTORY_DATA_START RECENT_DATA_SIZE          // 历史数据起始索引
#define HISTORY_DATA_SIZE  (TOTAL_SLAVES * RECORDS_PER_SLAVE)  // 历史数据大小(140条)

// 兼容旧代码的宏（避免编译报错）
#ifndef HISTORY_SIZE
#define HISTORY_SIZE      255     // 保留旧宏，兼容原有逻辑
#define PAGE7_ITEM_COUNT  5       // 保留PAGE7菜单项数量
#endif

// -------------------------- 硬件引脚定义 --------------------------
sbit CE = P5^4;  // 保留原有引脚定义

// -------------------------- 页面枚举（保留原有所有页面，兼容菜单逻辑） --------------------------
typedef enum {
    PAGE_1 = 0,       // 主页面
    PAGE_2 = 1,       // 功能菜单
    PAGE_3 = 2,       // 数据汇总
    PAGE_4 = 3,       // 从站统计
    PAGE_5 = 4,       // 参数设置
    PAGE_6 = 5,       // 系统信息
    PAGE_7 = 6,       // 事件查询
    PAGE_8 = 7,       // 校准页面
    PAGE_9 = 8,       // 扩展页面1
    PAGE_10 = 9,      // 扩展页面2
    PAGE_11 = 10,     // 扩展页面3
    PAGE_12 = 11      // 扩展页面4
} PageType;

// -------------------------- 菜单项枚举（保留原有定义，兼容菜单逻辑） --------------------------
typedef enum {
    // PAGE_2菜单项
    MENU_MEASURE_DATA = 0,        // 测量数据
    MENU_SENSOR_STATUS = 1,       // 传感器状态
    MENU_PARAM_QUERY = 2,         // 参数查询
    MENU_HISTORY_RECORD = 3,      // 历史记录
    MENU_DEVICE_MAINT = 4,        // 设备维护
    PAGE2_ITEM_COUNT = 5,         // PAGE2菜单项数量

    // PAGE_7菜单项
    MENU_SENSOR_EVENT_1 = 0,      // 传感器事件1
    MENU_SENSOR_EVENT_2 = 1,      // 传感器事件2
    MENU_SENSOR_EVENT_3 = 2,      // 传感器事件3
    MENU_HISTORY_DATA_QUERY = 3,  // 历史数据查询
    MENU_MAX_TEMP_QUERY = 4,      // 最高温度查询
    PAGE7_ITEM_COUNT = 5          // PAGE7菜单项数量
} MenuItemType;

// -------------------------- 核心数据结构体（替换旧的DataPoint，对齐uart4.c） --------------------------
// 数据记录结构体（分区存储核心）
typedef struct {
    unsigned char pid;                // 产品ID
    unsigned char aid;                // 从站ID
    unsigned char temp;               // 温度值
    unsigned char volt1;              // 电压1
    unsigned char volt2;              // 电压2
    unsigned char fosc;               // 频率值
    unsigned long timestamp;          // 时间戳(ms)
    unsigned char is_valid : 1;       // 数据有效标志(位域)
    unsigned char reserved : 7;       // 保留位
} DataRecord;

// 兼容旧代码的DataPoint结构体（避免原有代码报错）
typedef struct {
    unsigned char pid;
    unsigned char aid;
    unsigned char temp;
    unsigned char volt1;
    unsigned char volt2;
    unsigned char fosc;
    unsigned char voltage;  
} DataPoint;

// -------------------------- 统计信息结构体（统一定义，对齐uart4.c） --------------------------
typedef struct {
    unsigned char total_slaves;        // 在线从站数
    unsigned char total_data_count;    // 有效数据条数
    unsigned char overall_avg_temp;    // 整体平均温度
    unsigned char overall_max_temp;    // 整体最高温度
    unsigned char overall_min_temp;    // 整体最低温度
} Statistics;

// -------------------------- 菜单状态结构体（保留原有定义，兼容菜单逻辑） --------------------------
typedef struct {
    PageType current_page;
    PageType prev_page; 
    unsigned char page2_selected;
    unsigned char page7_selected;  // PAGE_7选中项
    unsigned char page10_selected;
    unsigned char page11_selected;
    unsigned char page12_selected;
    unsigned char page_changed;    
    unsigned char menu_initialized; 
} MenuState;

// -------------------------- 全局变量声明（对齐uart4.c + 保留原有变量） --------------------------
// 串口接收相关
extern unsigned char uart_rx_buff[UART_BUFF_SIZE];
extern unsigned char uart_rx_len;
extern bit uart_rx_complete;

// 页面/显示相关
extern PageType current_page;
extern unsigned long delay_count;
extern unsigned char pid_data[10];
extern unsigned char aid_data[10];
extern unsigned char temp_data[10];
extern unsigned char volt_data[10];
extern unsigned char fosc_data[10];
extern bit display_labels_initialized;

// 菜单状态（保留原有）
extern MenuState menu_state;

// 数据存储核心（对齐uart4.c）
extern DataRecord data_summary[TOTAL_RECORDS];          // 分区存储的总数据数组
extern unsigned char history_index[TOTAL_SLAVES];       // 每个从站的历史数据索引（数组形式）
extern Statistics current_stats;                        // 全局统计信息

// -------------------------- 函数声明（整合新旧，完全匹配uart4.c） --------------------------
// 串口4核心函数
void UART4_Init(unsigned long baudrate);
void UART4_SendByte(unsigned char dat);
void UART4_SendString(unsigned char *str);
void UART4_SendNumber(unsigned long num, unsigned char digits);
void UART4_ReceiveString(void);
void Debug_ShowParsedData(void);

// 定时器/延时函数
void Timer0_Init(void);
void delay_ms(unsigned long ms);

// LCD按键处理
void LCD_HandleKey(unsigned char key);

// 数据存储核心函数（对齐uart4.c）
void InitDataStorage(void);
void AddDataToSummary(unsigned char pid, unsigned char aid, unsigned char temp, 
                     unsigned char volt1, unsigned char volt2, unsigned char fosc);
DataRecord* GetRecentDataByAID(unsigned char aid);
void CalculateSummaryStatistics(void);
void UpdateSummaryDisplay(void);

// 辅助函数（保留+新增）
void ShowSlaveCount(void);
unsigned char CountActiveSlaves(void);
void ExportSummaryData(void);

// 原有菜单/兼容函数（保留声明，避免编译报错）
void Menu_Init(void);
void InitStatistics(void);
void AddDataToHistory(unsigned char pid, unsigned char aid, unsigned char temp, 
                     unsigned char volt1, unsigned char volt2, unsigned char fosc);
void CalculateStatistics(void);
void RefreshDisplay(void);
void ShowMaxTempQuery(void);
unsigned char CountDifferentSlaves(void);
void ExportHistoryData(void);

#endif // __UART4_H__