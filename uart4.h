#ifndef __UART4_H__
#define __UART4_H__

#include "STC32G.H"
#include "config.h"
#include "led.h"

// 核心分区存储宏定义
#define TOTAL_SLAVES      35       // 总从站数
#define RECORDS_PER_SLAVE 4        // 每个从站历史记录数
#define RECENT_RECORDS    1        // 每个从站最近记录数
#define TOTAL_RECORDS     (TOTAL_SLAVES * (RECENT_RECORDS + RECORDS_PER_SLAVE))  // 总记录数=175

// 分区索引定义
#define RECENT_DATA_START 0                          // 最近数据起始索引
#define RECENT_DATA_SIZE  TOTAL_SLAVES               // 最近数据大小(35条)
#define HISTORY_DATA_START RECENT_DATA_SIZE          // 历史数据起始索引
#define HISTORY_DATA_SIZE  (TOTAL_SLAVES * RECORDS_PER_SLAVE)  // 历史数据大小(140条)

// 兼容旧代码的宏
#ifndef HISTORY_SIZE
#define HISTORY_SIZE      255     
#define PAGE7_ITEM_COUNT  5       
#endif

// 预警记录和恢复记录最大数量
#define WARNING_RECORD_SIZE  10
#define RECOVERY_RECORD_SIZE 10

// 硬件引脚定义
sbit CE = P5^4;

// 页面枚举
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
    PAGE_10 = 9,      // 温度报警事件记录列表
    PAGE_11 = 10,     // 报警事件详细数据查看
    PAGE_12 = 11,     // 报警事件删除确认
    PAGE_13 = 12,     // 温度预警时间记录列表
    PAGE_14 = 13,     // 传感器恢复事件记录列表
    PAGE_15 = 14,     // 预警/恢复详细数据查看
    PAGE_16 = 15,     // 预警删除确认
    PAGE_17 = 16,     // 恢复删除确认
    PAGE_18 = 17,     // 新增：历史数据查询页面
    PAGE_19 = 18,     // 新增：清除历史数据确认页面
    PAGE_20 = 19,      // 新增：历史数据删除确认页面
	 PAGE_21 = 20,     // 新增：最高温度查询页面
    PAGE_22 = 21,     // 新增：最高温度清除确认页面
    PAGE_23 = 22,      // 新增：最高温度删除确认页面
		 PAGE_24 = 23,     // 修改日期时间详情
    PAGE_25 = 24,     // 修改密码详情
    PAGE_26 = 25      // 恢复出厂设置详情
} PageType;

// 菜单项枚举
typedef enum {
    // PAGE_2菜单项
    MENU_MEASURE_DATA = 0,        // 测量数据
    MENU_SENSOR_STATUS = 1,       // 传感器状态
    MENU_PARAM_QUERY = 2,         // 参数查询
    MENU_HISTORY_RECORD = 3,      // 历史记录
    MENU_DEVICE_MAINT = 4,        // 设备维护
    PAGE2_ITEM_COUNT = 5,         // PAGE2菜单项数量

    // PAGE_7菜单项
    MENU_SENSOR_EVENT_1 = 0,      // 温度报警事件记录
    MENU_SENSOR_EVENT_2 = 1,      // 温度预警时间记录
    MENU_SENSOR_EVENT_3 = 2,      // 传感器恢复事件记录
    MENU_HISTORY_DATA_QUERY = 3,  // 历史数据查询
    MENU_MAX_TEMP_QUERY = 4,      // 最高温度查询
    PAGE7_ITEM_COUNT = 5,         // PAGE7菜单项数量
	
	 MENU_DATE_TIME = 10,          // 修改日期时间
    MENU_PASSWORD = 11,           // 修改密码
    MENU_FACTORY_RESET = 12,      // 恢复出厂设置
    PAGE8_ITEM_COUNT = 3          // PAGE8菜单项数量
} MenuItemType;

// 数据记录结构体
typedef struct {
    unsigned char pid;                // 产品ID
    unsigned char aid;                // 从站ID
    unsigned char temp;               // 温度值
    unsigned char volt1;              // 电压1
    unsigned char volt2;              // 电压2
    unsigned char fosc;               // 频率值
    unsigned long timestamp;          // 时间戳(ms)
    unsigned char is_valid;           // 数据有效标志(1字节)
    unsigned char reserved[3];        // 保留字节
} DataRecord;

// 兼容旧代码的DataPoint结构体
typedef struct {
    unsigned char pid;
    unsigned char aid;
    unsigned char temp;
    unsigned char volt1;
    unsigned char volt2;
    unsigned char fosc;
    unsigned char voltage;  
} DataPoint;

// 预警记录数据结构
typedef struct {
    unsigned char id;           // 记录ID
    unsigned char sensor_id;    // 传感器ID
    unsigned char temperature;  // 预警温度
    unsigned char time_hour;    // 预警时间-小时
    unsigned char time_minute;  // 预警时间-分钟
    unsigned char is_valid;     // 记录是否有效
} WarningRecord;

// 恢复记录数据结构
typedef struct {
    unsigned char id;           // 记录ID
    unsigned char sensor_id;    // 传感器ID
    unsigned char time_hour;    // 恢复时间-小时
    unsigned char time_minute;  // 恢复时间-分钟
    unsigned char status;       // 恢复状态
    unsigned char is_valid;     // 记录是否有效
} RecoveryRecord;

// 统计信息结构体
typedef struct {
    unsigned char total_slaves;        // 在线从站数
    unsigned char total_data_count;    // 有效数据条数
    unsigned char overall_avg_temp;    // 整体平均温度
    unsigned char overall_max_temp;    // 整体最高温度
    unsigned char overall_min_temp;    // 整体最低温度
} Statistics;

// 菜单状态结构体
typedef struct {
    PageType current_page;
    PageType prev_page; 
    unsigned char page2_selected;
    unsigned char page7_selected;   // PAGE_7选中项
	unsigned char page8_selected;   // PAGE_7选中项
    unsigned char page10_selected;  // PAGE_10选中项
    unsigned char page11_selected;  // PAGE_11选中项
    unsigned char page12_selected;  // PAGE_12选中项
    unsigned char page13_selected;  // PAGE_13选中项
    unsigned char page14_selected;  // PAGE_14选中项
    unsigned char page15_selected;  // PAGE_15选中项
    unsigned char page16_selected;  // PAGE_16选中项
    unsigned char page17_selected;  // PAGE_17选中项
	  unsigned char page18_selected;  // PAGE_18选中项
    unsigned char page19_selected;  // PAGE_19选中项
    unsigned char page20_selected;  // PAGE_20选中项
		  unsigned char page21_selected;  // PAGE_18选中项
    unsigned char page22_selected;  // PAGE_19选中项
    unsigned char page23_selected;  // PAGE_20选中项
	    unsigned char page24_selected;  // PAGE_24选中项 - 新增
    unsigned char page25_selected;  // PAGE_25选中项 - 新增
    unsigned char page26_selected;  // PAGE_26选中项 - 新增
    unsigned char page_changed;    
    unsigned char menu_initialized; 
} MenuState;

// 全局变量声明
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

// 菜单状态
extern MenuState menu_state;

// 数据存储核心
extern DataRecord data_summary[TOTAL_RECORDS];          // 分区存储的总数据数组
extern unsigned char slave_history_index[TOTAL_SLAVES]; // 每个从站的历史数据索引
extern Statistics current_stats;                        // 全局统计信息

// 预警和恢复记录数组
extern WarningRecord warning_records[WARNING_RECORD_SIZE];
extern RecoveryRecord recovery_records[RECOVERY_RECORD_SIZE];

// 历史数据数组（兼容旧代码）
extern DataPoint history[HISTORY_SIZE];

// 函数声明
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

// 数据存储核心函数
void InitDataStorage(void);
void AddDataToSummary(unsigned char pid, unsigned char aid, unsigned char temp, 
                     unsigned char volt1, unsigned char volt2, unsigned char fosc);
DataRecord* GetRecentDataByAID(unsigned char aid);
void CalculateStatistics(void);
void UpdateSummaryDisplay(void);

// 辅助函数
void ShowSlaveCount(void);
unsigned char CountActiveSlaves(void);
void ExportSummaryData(void);

// 原有菜单/兼容函数
void Menu_Init(void);
void InitStatistics(void);
void AddDataToHistory(unsigned char pid, unsigned char aid, unsigned char temp, 
                     unsigned char volt1, unsigned char volt2, unsigned char fosc);
unsigned char CountDifferentSlaves(void);
void ExportHistoryData(void);
void RefreshDisplay(void);
void ShowMaxTempQuery(void);

// 新增预警/恢复记录相关函数
void InitWarningRecords(void);
void InitRecoveryRecords(void);
void DeleteWarningRecord(unsigned char index);
void DeleteRecoveryRecord(unsigned char index);

#endif // __UART4_H__
