/*
 * STC8G1K08 从机配置
 */

#ifndef SLAVE_CONFIG_H
#define SLAVE_CONFIG_H

#include <STC8G1K08.h>

// ==================== 节点配置 ====================
#define NODE_ID         0x01    // 默认节点地址 (可通过拨码开关设置)

// ==================== 通信配置 ====================
#define SEND_INTERVAL   5       // 心跳发送间隔 (秒)
#define PAN3031_FREQ    434000000  // 频率 434MHz
#define PAN3031_SF      7       // 扩频因子
#define PAN3031_BW      125000  // 带宽 125kHz
#define PAN3031_PWR     20      // 发射功率 20dBm

// ==================== 命令字定义 ====================
#define CMD_HEARTBEAT   0x01    // 心跳包
#define CMD_QUERY       0x02    // 查询状态
#define CMD_PUMP_CTRL   0x10    // 水泵控制
#define CMD_SET_AUTO    0x20    // 自动模式
#define CMD_SET_MANUAL  0x21    // 手动模式
#define CMD_ALARM       0xFF    // 报警

// ==================== 功耗配置 ====================
// 睡眠模式：
// - CPU 停止
// - ADC 关闭
// - PAN3031 睡眠
// - 仅外部中断工作
// 睡眠电流：<10μA

// 工作模式：
// - CPU 运行
// - ADC 采样
// - PAN3031 监听
// 工作电流：~5mA

// ==================== 数据类型 ====================
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;
typedef bit bool;

#endif
