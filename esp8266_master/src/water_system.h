/*
 * 水塔监控系统 - 数据结构定义
 */

#ifndef WATER_SYSTEM_H
#define WATER_SYSTEM_H

#include <stdint.h>

// 最大水塔数量
#define MAX_TOWERS 8

// 历史记录数量 (每个水塔保存最近 48 小时，每小时 1 条)
#define HISTORY_SIZE 48

// 命令字定义
#define CMD_HEARTBEAT   0x01  // 心跳包
#define CMD_QUERY       0x02  // 查询水位
#define CMD_PUMP_CTRL   0x10  // 水泵控制
#define CMD_SET_AUTO    0x20  // 自动模式
#define CMD_SET_MANUAL  0x21  // 手动模式
#define CMD_ALARM       0xFF  // 报警

// 系统模式
typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL = 1
} SystemMode;

// 历史记录
typedef struct {
    uint32_t timestamp;    // 时间戳 (秒)
    uint8_t water_level;   // 水位百分比
    bool pump_status;      // 水泵状态
} HistoryRecord;

// 水塔数据
typedef struct {
    uint8_t id;              // 水塔 ID (从机地址)
    uint8_t water_level;     // 水位百分比 (0-100)
    bool pump_on;            // 水泵状态
    bool online;             // 在线状态
    bool low_water_alarm;    // 低水位报警
    bool overflow_alarm;     // 溢水报警
    bool shortage_alarm;     // 缺水报警
    uint32_t last_update;    // 最后更新时间
    char name[16];           // 水塔名称
    HistoryRecord history[HISTORY_SIZE];  // 历史记录
    uint8_t history_index;   // 历史记录索引
} TowerData;

// 系统状态
typedef struct {
    SystemMode mode;         // 系统模式
    bool wifi_connected;     // WiFi 连接状态
    bool well_water_ok;      // 井水正常
    uint32_t last_heartbeat; // 最后心跳时间
    uint32_t last_save;      // 最后保存时间
} SystemStatus;

// 控制命令 (来自 APP)
typedef struct {
    uint8_t tower_id;        // 水塔 ID
    bool pump_on;            // 水泵开关
    bool auto_mode;          // 自动模式
} ControlCommand;

#endif // WATER_SYSTEM_H
