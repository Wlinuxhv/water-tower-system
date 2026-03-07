/*
 * 水塔监控从机节点 - STC8G1K08 简化版
 * 功能：
 * 1. 读取液位传感器 (ADC)
 * 2. 检测缺水
 * 3. 控制水泵继电器
 * 4. 通过 PAN3031 与主机通信
 */

#include <8051.h>
#include "pan3031.h"
#include "slave_config.h"

// 引脚定义
sbit PAN3031_CS   = P2^3;
sbit PAN3031_MOSI = P2^2;
sbit PAN3031_MISO = P2^1;
sbit PAN3031_SCK  = P2^0;
sbit PUMP_RELAY   = P3^3;
sbit WATER_LOW_DET = P3^4;

// 全局变量
volatile unsigned char node_id = NODE_ID;
volatile unsigned char water_level = 0;
volatile unsigned char pump_on = 0;
volatile unsigned char well_water_ok = 1;
volatile unsigned long last_send = 0;

// 函数声明
void system_init(void);
unsigned char read_water_level(void);
unsigned char check_well_water(void);
void pump_control(unsigned char on);
void send_heartbeat(void);
void delay_ms(unsigned int ms);
unsigned long millis(void);

// 主函数
void main(void) {
    system_init();
    
    // 发送上电信号
    send_heartbeat();
    
    while (1) {
        // 读取水位 (简化：模拟值)
        water_level = read_water_level();
        
        // 检查缺水
        well_water_ok = check_well_water();
        
        // 定期发送心跳 (每 5 秒)
        if (millis() - last_send > 5000) {
            send_heartbeat();
        }
        
        delay_ms(100);
    }
}

// 系统初始化
void system_init(void) {
    // GPIO 初始化
    P0 = 0xFF;
    P1 = 0xFF;
    P2 = 0xFF;
    P3 = 0xFF;
    
    // PAN3031 初始化
    pan3031_init();
    pan3031_set_freq(434000000UL);
    pan3031_set_sf(7);
    pan3031_set_bw(125000UL);
    pan3031_set_power(20);
    
    // 关闭水泵
    pump_control(0);
}

// 读取水位 (简化版)
unsigned char read_water_level(void) {
    static unsigned char sim_level = 50;
    sim_level++;
    if (sim_level > 100) sim_level = 0;
    return sim_level;
}

// 检查缺水
unsigned char check_well_water(void) {
    return (WATER_LOW_DET == 1) ? 1 : 0;
}

// 水泵控制
void pump_control(unsigned char on) {
    PUMP_RELAY = on ? 1 : 0;
    pump_on = on;
}

// 发送心跳
void send_heartbeat(void) {
    unsigned char tx_data[5];
    
    tx_data[0] = node_id;
    tx_data[1] = CMD_HEARTBEAT;
    tx_data[2] = 2;
    tx_data[3] = water_level;
    tx_data[4] = well_water_ok ? 1 : 0;
    
    pan3031_send(tx_data, 5);
    last_send = millis();
}

// 延时函数
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 1200; j++);
    }
}

// 毫秒计时
unsigned long millis(void) {
    static unsigned long count = 0;
    return count++;
}
