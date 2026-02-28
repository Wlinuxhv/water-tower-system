/*
 * SC09B 9 通道水位检测芯片驱动头文件
 */

#ifndef SC09B_H
#define SC09B_H

#include "slave_config.h"

// ==================== SC09B 配置 ====================

// I2C 地址 (SC09B 固定地址)
// 简易 I2C 协议：写地址 = 0xA0, 读地址 = 0xA1
#define SC09B_ADDR_WRITE  0xA0
#define SC09B_ADDR_READ   0xA1

// 水位通道定义
#define SC09B_CH1  0x0001  // 10% 水位
#define SC09B_CH2  0x0002  // 20% 水位
#define SC09B_CH3  0x0004  // 30% 水位
#define SC09B_CH4  0x0008  // 40% 水位
#define SC09B_CH5  0x0010  // 50% 水位
#define SC09B_CH6  0x0020  // 60% 水位
#define SC09B_CH7  0x0040  // 70% 水位
#define SC09B_CH8  0x0080  // 80% 水位
#define SC09B_CH9  0x0100  // 90% 水位

// ==================== 函数声明 ====================

// 初始化
uint8_t sc09b_init(void);

// 读取原始数据 (9 位)
uint16_t sc09b_read_water_level(void);

// 读取状态寄存器
uint8_t sc09b_read_status(void);

// 获取水位百分比 (0-100%)
uint8_t sc09b_get_water_percent(void);

// 低功耗模式
void sc09b_sleep(void);
void sc09b_wakeup(void);

// 数据就绪检查
bit sc09b_data_ready(void);

#endif
