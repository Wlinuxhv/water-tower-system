/*
 * 水塔监控从机节点 - 简化版
 * 功能：
 * 1. 读取液位传感器
 * 2. 检测缺水
 * 3. 控制水泵
 * 4. 通过 PAN3031 与主机通信
 */

#include <8051.h>

// PAN3031 引脚定义
__sbit __at (0xA3) PAN3031_CS;    // P2.3
__sbit __at (0xA2) PAN3031_MOSI;  // P2.2
__sbit __at (0xA1) PAN3031_MISO;  // P2.1
__sbit __at (0xA0) PAN3031_SCK;   // P2.0

// 其他引脚
__sbit __at (0xB3) PUMP_RELAY;    // P3.3
__sbit __at (0xB4) WATER_LOW_DET; // P3.4

#include "pan3031.h"
#include "water_slave.h"

// ==================== 全局变量 ====================
volatile unsigned char node_id = NODE_ID;
volatile unsigned char water_level = 0;
volatile unsigned char pump_on = 0;
volatile unsigned char well_water_ok = 1;
volatile unsigned long last_send = 0;

// ==================== 函数声明 ====================
void system_init(void);
unsigned char read_water_level(void);
unsigned char check_well_water(void);
void pump_control(unsigned char on);
void send_heartbeat(void);
void handle_command(unsigned char *data, unsigned char len);

// ==================== 主函数 ====================
void main(void) {
    system_init();
    
    // 发送上电信号
    send_heartbeat();
    
    while (1) {
        // 读取水位 (简化：使用模拟值)
        water_level = read_water_level();
        
        // 检查缺水
        well_water_ok = check_well_water();
        
        // 处理接收命令 (简化：不实现)
        // uint8_t rx_data[32];
        // uint8_t rx_len;
        // if (pan3031_receive(rx_data, &rx_len)) {
        //     handle_command(rx_data, rx_len);
        // }
        
        // 定期发送心跳 (每 5 秒)
        if (millis() - last_send > 5000) {
            send_heartbeat();
        }
        
        delay_ms(100);
    }
}

// ==================== 系统初始化 ====================
void system_init(void) {
    // GPIO 初始化 (8051 默认准双向，无需特别配置)
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

// ==================== 读取水位 ====================
unsigned char read_water_level(void) {
    // 简化实现：使用固定值模拟
    static unsigned char sim_level = 50;
    sim_level++;
    if (sim_level > 100) sim_level = 0;
    return sim_level;
}

// ==================== 检查缺水 ====================
unsigned char check_well_water(void) {
    return (WATER_LOW_DET == 1) ? 1 : 0;
}

// ==================== 水泵控制 ====================
void pump_control(unsigned char on) {
    PUMP_RELAY = on ? 1 : 0;
    pump_on = on;
}

// ==================== 发送心跳 ====================
void send_heartbeat(void) {
    unsigned char tx_data[5];
    
    tx_data[0] = node_id;          // 源地址
    tx_data[1] = CMD_HEARTBEAT;    // 命令字
    tx_data[2] = 2;                // 数据长度
    tx_data[3] = water_level;
    tx_data[4] = well_water_ok ? 1 : 0;
    
    pan3031_send(tx_data, 5);
    last_send = millis();
}

// ==================== 处理命令 ====================
void handle_command(unsigned char *data, unsigned char len) {
    unsigned char src_id, cmd, data_len;
    
    if (len < 3) return;
    
    src_id = data[0];
    cmd = data[1];
    data_len = data[2];
    
    // 只响应主机命令
    if (src_id != 0x00) return;
    
    switch (cmd) {
        case CMD_PUMP_CTRL:
            if (data_len >= 1) {
                pump_control(data[3] ? 1 : 0);
            }
            break;
            
        case CMD_QUERY:
            send_heartbeat();
            break;
    }
}
