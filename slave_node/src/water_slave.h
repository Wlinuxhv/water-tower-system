/*
 * 水塔监控系统 - 从机数据结构定义
 */

#ifndef WATER_SLAVE_H
#define WATER_SLAVE_H

#include <stdint.h>

// 节点 ID (通过拨码开关设置)
#define NODE_ID 0x01

// 发送间隔 (毫秒)
#define SEND_INTERVAL 5000

// 节点状态
typedef struct {
    uint8_t node_id;         // 节点 ID
    uint8_t water_level;     // 水位百分比 (0-100)
    uint8_t pump_on;         // 水泵状态 (0/1)
    uint8_t well_water_ok;   // 井水状态 (1=正常，0=缺水)
    uint32_t last_send;      // 最后发送时间
} NodeStatus;

#endif // WATER_SLAVE_H
