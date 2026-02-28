/*
 * SC09B 9 通道水位检测芯片驱动
 * 厦门晶尊微电子 (ICman)
 * 
 * 特性:
 * - 9 通道电容式水位检测
 * - I2C 接口 (简易 I2C)
 * - 低功耗：工作 0.65mA，睡眠 20μA
 * - 工作电压：2.5V-6.5V
 * 
 * 引脚连接:
 * - SCL: P3.0
 * - SDA: P3.1
 * - INT: P3.2 (可选)
 */

#include "sc09b.h"
#include <STC8G1K08.h>

// ==================== I2C 引脚定义 ====================
sbit SC09B_SCL = P3^0;
sbit SC09B_SDA = P3^1;
sbit SC09B_INT = P3^2;  // 可选中断引脚

// ==================== I2C 基础函数 ====================

// 短延时 (约 1μs @ 12MHz)
static void i2c_delay(void) {
    _nop_();
    _nop_();
    _nop_();
}

// I2C 初始化
void i2c_init(void) {
    SC09B_SCL = 1;
    SC09B_SDA = 1;
    
    // P3.0, P3.1 设为准双向口
    P3M0 &= ~0x03;
    P3M1 &= ~0x03;
}

// I2C 起始信号
static void i2c_start(void) {
    SC09B_SDA = 1;
    i2c_delay();
    SC09B_SCL = 1;
    i2c_delay();
    SC09B_SDA = 0;
    i2c_delay();
    SC09B_SCL = 0;
    i2c_delay();
}

// I2C 停止信号
static void i2c_stop(void) {
    SC09B_SDA = 0;
    i2c_delay();
    SC09B_SCL = 1;
    i2c_delay();
    SC09B_SDA = 1;
    i2c_delay();
}

// I2C 发送一个字节
static bit i2c_send_byte(uint8_t data) {
    uint8_t i;
    bit ack;
    
    for (i = 0; i < 8; i++) {
        if (data & 0x80)
            SC09B_SDA = 1;
        else
            SC09B_SDA = 0;
        data <<= 1;
        
        i2c_delay();
        SC09B_SCL = 1;
        i2c_delay();
        SC09B_SCL = 0;
        i2c_delay();
    }
    
    // 读取应答
    SC09B_SDA = 1;
    i2c_delay();
    SC09B_SCL = 1;
    i2c_delay();
    ack = SC09B_SDA;
    SC09B_SCL = 0;
    i2c_delay();
    
    return ack;
}

// I2C 读取一个字节
static uint8_t i2c_read_byte(bit ack) {
    uint8_t i, data = 0;
    
    SC09B_SDA = 1;
    
    for (i = 0; i < 8; i++) {
        i2c_delay();
        SC09B_SCL = 1;
        i2c_delay();
        data <<= 1;
        if (SC09B_SDA)
            data |= 0x01;
        SC09B_SCL = 0;
        i2c_delay();
    }
    
    // 发送应答
    if (ack)
        SC09B_SDA = 0;
    else
        SC09B_SDA = 1;
    
    i2c_delay();
    SC09B_SCL = 1;
    i2c_delay();
    SC09B_SCL = 0;
    i2c_delay();
    SC09B_SDA = 1;
    
    return data;
}

// ==================== SC09B 驱动函数 ====================

/**
 * SC09B 初始化
 * 返回：0=成功，1=失败
 */
uint8_t sc09b_init(void) {
    i2c_init();
    
    // 读取芯片状态验证
    uint8_t status = sc09b_read_status();
    
    // 检查是否在线 (简单验证)
    if (status == 0xFF) {
        return 1;  // 通信失败
    }
    
    return 0;  // 成功
}

/**
 * 读取 SC09B 状态寄存器
 */
uint8_t sc09b_read_status(void) {
    uint8_t status;
    
    i2c_start();
    i2c_send_byte(SC09B_ADDR_WRITE);
    i2c_stop();
    
    i2c_start();
    i2c_send_byte(SC09B_ADDR_READ);
    status = i2c_read_byte(0);  // 无应答
    i2c_stop();
    
    return status;
}

/**
 * 读取 9 个通道的水位状态
 * 返回值：9 位数据，每位代表一个通道
 *         bit0=通道 1(10%), bit1=通道 2(20%)...bit8=通道 9(90%)
 */
uint16_t sc09b_read_water_level(void) {
    uint8_t data_l, data_h;
    uint16_t result;
    
    i2c_start();
    i2c_send_byte(SC09B_ADDR_WRITE);
    i2c_stop();
    
    i2c_start();
    i2c_send_byte(SC09B_ADDR_READ);
    
    // 读取低 8 位 (通道 1-8)
    data_l = i2c_read_byte(1);  // 有应答
    // 读取高 1 位 (通道 9)
    data_h = i2c_read_byte(0);  // 无应答
    
    i2c_stop();
    
    result = ((uint16_t)data_h << 8) | data_l;
    
    return result;
}

/**
 * 计算水位百分比 (0-100%)
 * 返回：水位百分比
 */
uint8_t sc09b_get_water_percent(void) {
    uint16_t channels = sc09b_read_water_level();
    uint8_t level = 0;
    
    // 从高位到低位检查
    if (channels & 0x0100) level = 95;      // 通道 9 (90%)
    else if (channels & 0x0080) level = 85; // 通道 8 (80%)
    else if (channels & 0x0040) level = 75; // 通道 7 (70%)
    else if (channels & 0x0020) level = 65; // 通道 6 (60%)
    else if (channels & 0x0010) level = 55; // 通道 5 (50%)
    else if (channels & 0x0008) level = 45; // 通道 4 (40%)
    else if (channels & 0x0004) level = 35; // 通道 3 (30%)
    else if (channels & 0x0002) level = 25; // 通道 2 (20%)
    else if (channels & 0x0001) level = 15; // 通道 1 (10%)
    else level = 5;                          // 无水
    
    return level;
}

/**
 * 进入省电模式
 * SC09B 睡眠电流：20μA
 */
void sc09b_sleep(void) {
    // SC09B 自动进入省电模式
    // 当没有触摸/水位检测时自动睡眠
    // 无需特殊命令
}

/**
 * 唤醒 SC09B
 */
void sc09b_wakeup(void) {
    // 发送任意 I2C 命令唤醒
    i2c_start();
    i2c_send_byte(SC09B_ADDR_WRITE);
    i2c_stop();
}

/**
 * 检查是否有数据就绪 (通过 INT 引脚)
 * 返回：1=数据就绪，0=无数据
 */
bit sc09b_data_ready(void) {
    return (SC09B_INT == 0);  // 低电平表示数据就绪
}
