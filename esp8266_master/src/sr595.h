/*
 * 74HC595 移位寄存器驱动 - 8 路继电器控制
 * 
 * 硬件连接:
 * - ESP8266 D5 (SCK)  → 74HC595 SH_CP (Pin 11)
 * - ESP8266 D7 (MOSI) → 74HC595 DS (Pin 14)
 * - ESP8266 D4 (GPIO2) → 74HC595 ST_CP (Pin 12, 锁存)
 * - ESP8266 D8 (CS)   → LoRa CS (与 74HC595 无关)
 * 
 * 特性:
 * - 与 LoRa 共用 SPI 总线 (SCK, MOSI)
 * - 独立锁存引脚控制
 * - 支持 8 路继电器独立同时控制
 * - 支持级联扩展 (最多 64 路)
 */

#ifndef SR595_H
#define SR595_H

#include <Arduino.h>

// ==================== 引脚定义 ====================
#define SR_LATCH_PIN  D4      // GPIO2 - 74HC595 锁存引脚
#define LORA_CS_PIN   D8      // GPIO15 - LoRa 片选 (用于互斥控制)

// ==================== 全局变量 ====================
extern uint8_t g_relay_state;  // 继电器当前状态

// ==================== 函数声明 ====================
/**
 * 初始化 74HC595
 * 配置锁存引脚和 LoRa CS 引脚
 */
void sr595_init(void);

/**
 * 写入 8 位数据到 74HC595
 * @param data 8 位数据，每位控制一个继电器
 */
void sr595_write(uint8_t data);

/**
 * 开启指定继电器
 * @param relay_id 继电器编号 (0-7)
 */
void sr595_relay_on(uint8_t relay_id);

/**
 * 关闭指定继电器
 * @param relay_id 继电器编号 (0-7)
 */
void sr595_relay_off(uint8_t relay_id);

/**
 * 设置所有继电器状态
 * @param mask 8 位掩码，1=开启，0=关闭
 */
void sr595_set_all(uint8_t mask);

/**
 * 获取当前继电器状态
 * @return 8 位状态值
 */
uint8_t sr595_get_state(void);

/**
 * 切换指定继电器状态
 * @param relay_id 继电器编号 (0-7)
 */
void sr595_toggle(uint8_t relay_id);

/**
 * 脉冲控制继电器 (用于测试)
 * @param relay_id 继电器编号
 * @param delay_ms 脉冲宽度 (毫秒)
 */
void sr595_pulse(uint8_t relay_id, uint16_t delay_ms);

#endif  // SR595_H
