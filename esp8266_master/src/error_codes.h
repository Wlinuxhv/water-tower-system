/*
 * 系统错误码定义
 * 
 * 错误码格式：ERR_XXX_YYY
 * - XXX: 错误类别 (SYS, SEN, COM, REL, NET, WDT)
 * - YYY: 具体错误编号 (001-999)
 * 
 * 显示格式：Exx-yyy (OLED 显示 8 字符)
 */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <Arduino.h>

// ==================== 错误码定义 ====================

// 系统错误 (001-099)
#define ERR_SYS_OK              0       // 系统正常
#define ERR_SYS_INIT_FAIL       1       // 系统初始化失败
#define ERR_SYS_MEMORY_LOW      2       // 内存不足
#define ERR_SYS_WDT_RESET       3       // 看门狗复位
#define ERR_SYS_POWER_LOW       4       // 电源电压低
#define ERR_SYS_OVERHEAT        5       // 系统过热

// 传感器错误 (100-199)
#define ERR_SENSOR_OK           0       // 传感器正常
#define ERR_SENSOR_WATER_LOW    101     // 井水缺水
#define ERR_SENSOR_ADC_FAIL     102     // ADC 读取失败
#define ERR_SENSOR_OUT_OF_RANGE 103     // 传感器数据超范围
#define ERR_SENSOR_DISCONNECT   104     // 传感器断开

// 通信错误 (200-299)
#define ERR_COM_OK              0       // 通信正常
#define ERR_COM_LORA_TIMEOUT    201     // LoRa 通信超时
#define ERR_COM_LORA_CRC        202     // LoRa CRC 校验失败
#define ERR_COM_I2C_FAIL        203     // I2C 通信失败
#define ERR_COM_SPI_FAIL        204     // SPI 通信失败
#define ERR_COM_WIFI_FAIL       205     // WiFi 连接失败
#define ERR_COM_WEB_FAIL        206     // Web 服务器错误

// 继电器错误 (300-399)
#define ERR_REL_OK              0       // 继电器正常
#define ERR_REL_STUCK_ON        301     // 继电器粘连 (常开)
#define ERR_REL_STUCK_OFF       302     // 继电器卡死 (常闭)
#define ERR_REL_OVERLOAD        303     // 继电器过载
#define ERR_REL_DRIVER_FAIL     304     // 继电器驱动失败 (74HC595)

// 网络错误 (400-499)
#define ERR_NET_OK              0       // 网络正常
#define ERR_NET_NO_FROM         401     // 无来自从机数据
#define ERR_NET_TIMEOUT         402     // 网络超时
#define ERR_NET_INVALID_DATA    403     // 无效数据包

// 水塔错误 (500-599)
#define ERR_TOWER_OK            0       // 水塔正常
#define ERR_TOWER_OVERFLOW      501     // 水塔溢出
#define ERR_TOWER_DRY           502     // 水塔干涸
#define ERR_TOWER_PUMP_FAIL     503     // 水泵故障
#define ERR_TOWER_SENSOR_FAIL   504     // 水塔传感器故障

// ==================== 错误级别 ====================

typedef enum {
    ERR_LEVEL_NONE = 0,     // 无错误
    ERR_LEVEL_INFO = 1,     // 提示信息
    ERR_LEVEL_WARNING = 2,  // 警告
    ERR_LEVEL_ERROR = 3,    // 错误
    ERR_LEVEL_CRITICAL = 4  // 严重错误
} ErrorLevel_t;

// ==================== 错误结构体 ====================

typedef struct {
    uint16_t code;          // 错误码
    ErrorLevel_t level;     // 错误级别
    const char* message;    // 错误描述
    uint32_t timestamp;     // 发生时间 (毫秒)
    uint8_t tower_id;       // 相关水塔 ID (0xFF 表示系统级)
} Error_t;

// ==================== 全局错误日志 ====================

#define MAX_ERROR_LOG   10  // 最多保存 10 条错误

extern Error_t g_error_log[MAX_ERROR_LOG];
extern uint8_t g_error_count;
extern uint16_t g_current_error;  // 当前错误码

// ==================== 函数声明 ====================

/**
 * 初始化错误系统
 */
void error_init(void);

/**
 * 记录错误
 * @param code 错误码
 * @param level 错误级别
 * @param tower_id 相关水塔 ID (0xFF 表示系统级)
 */
void error_log(uint16_t code, ErrorLevel_t level, uint8_t tower_id);

/**
 * 清除错误
 * @param code 错误码 (0 表示清除所有)
 */
void error_clear(uint16_t code);

/**
 * 获取当前错误码
 * @return 当前错误码
 */
uint16_t error_get_current(void);

/**
 * 获取错误描述
 * @param code 错误码
 * @return 错误描述字符串
 */
const char* error_get_message(uint16_t code);

/**
 * 获取错误级别
 * @param code 错误码
 * @return 错误级别
 */
ErrorLevel_t error_get_level(uint16_t code);

/**
 * 在 OLED 上显示错误
 * @param code 错误码
 */
void error_display_oled(uint16_t code);

/**
 * 检查是否有错误
 * @return true=有错误，false=正常
 */
bool error_has_error(void);

/**
 * 检查是否有严重错误
 * @return true=有严重错误，false=正常
 */
bool error_has_critical(void);

/**
 * 获取错误日志
 * @param index 日志索引 (0-9)
 * @return 错误结构体指针
 */
Error_t* error_get_log(uint8_t index);

/**
 * 打印错误信息到串口
 * @param code 错误码
 */
void error_print_serial(uint16_t code);

/**
 * 系统健康检查
 * @return ERR_SYS_OK=健康，其他=故障
 */
uint16_t system_health_check(void);

#endif  // ERROR_CODES_H
