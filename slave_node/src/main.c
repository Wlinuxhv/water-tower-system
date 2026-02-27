/*
 * 水塔监控从机节点
 * 功能:
 * 1. 读取液位传感器
 * 2. 检测缺水
 * 3. 控制水泵
 * 4. 通过 PAN3031 与主机通信
 */

#include <STC32G.h>
#include "pan3031.h"
#include "water_slave.h"

// ==================== 引脚定义 ====================
// PAN3031
#define PAN3031_CS   P2_3
#define PAN3031_MOSI P2_2
#define PAN3031_MISO P2_1
#define PAN3031_SCK  P2_0
#define PAN3031_IRQ  P3_2

// 液位传感器 (ADC 输入)
#define LEVEL_SENSOR_ADC P1_0

// 水泵继电器
#define PUMP_RELAY     P3_3

// 缺水检测 (数字输入)
#define WATER_LOW_DET  P3_4

// 本地节点 ID (通过跳线帽设置)
#define NODE_ID        0x01

// ==================== 全局变量 ====================
NodeStatus node_status = {
    .node_id = NODE_ID,
    .water_level = 0,
    .pump_on = false,
    .well_water_ok = true,
    .last_send = 0
};

// ==================== 函数声明 ====================
void system_init();
uint8_t read_water_level();
bool check_well_water();
void pump_control(bool on);
void send_heartbeat();
void handle_command(uint8_t *data, uint8_t len);

// ==================== 主函数 ====================
void main(void) {
    system_init();
    
    // 发送上电信号
    send_heartbeat();
    
    while (1) {
        // 读取水位
        node_status.water_level = read_water_level();
        
        // 检查缺水
        node_status.well_water_ok = check_well_water();
        
        // 处理接收命令
        uint8_t rx_data[32];
        uint8_t rx_len;
        if (pan3031_receive(rx_data, &rx_len)) {
            handle_command(rx_data, rx_len);
        }
        
        // 定期发送心跳 (每 5 秒)
        if (millis() - node_status.last_send > 5000) {
            send_heartbeat();
        }
        
        delay_ms(100);
    }
}

// ==================== 系统初始化 ====================
void system_init() {
    // GPIO 初始化
    P0M0 = 0x00; P0M1 = 0x00;
    P1M0 = 0x00; P1M1 = 0x00;
    P2M0 = 0x08; P2M1 = 0x00;  // PAN3031 CS 输出
    P3M0 = 0x08; P3M1 = 0x00;  // 水泵继电器输出
    
    // PAN3031 初始化
    pan3031_init();
    pan3031_set_freq(434000000);
    pan3031_set_sf(7);
    pan3031_set_bw(125000);
    pan3031_set_power(20);
    
    // ADC 初始化 (液位传感器)
    // STC32G ADC 配置...
    
    // 关闭水泵
    pump_control(false);
}

// ==================== 读取水位 ====================
uint8_t read_water_level() {
    // 读取 ADC 值 (0-4095)
    uint16_t adc_value = adc_read(LEVEL_SENSOR_ADC);
    
    // 转换为百分比 (0-100%)
    uint8_t level = (uint8_t)(adc_value * 100 / 4095);
    
    return level;
}

// ==================== 检查缺水 ====================
bool check_well_water() {
    return (WATER_LOW_DET == 1);  // 高电平表示有水
}

// ==================== 水泵控制 ====================
void pump_control(bool on) {
    PUMP_RELAY = on ? 1 : 0;
    node_status.pump_on = on;
}

// ==================== 发送心跳 ====================
void send_heartbeat() {
    uint8_t tx_data[5];
    tx_data[0] = node_status.node_id;  // 源地址
    tx_data[1] = CMD_HEARTBEAT;        // 命令字
    tx_data[2] = 2;                    // 数据长度
    tx_data[3] = node_status.water_level;
    tx_data[4] = node_status.well_water_ok ? 1 : 0;
    
    pan3031_send(tx_data, 5);
    node_status.last_send = millis();
}

// ==================== 处理命令 ====================
void handle_command(uint8_t *data, uint8_t len) {
    if (len < 3) return;
    
    uint8_t src_id = data[0];
    uint8_t cmd = data[1];
    uint8_t data_len = data[2];
    
    // 只响应主机命令
    if (src_id != 0x00) return;
    
    switch (cmd) {
        case CMD_PUMP_CTRL:
            if (data_len >= 1) {
                pump_control(data[3] ? true : false);
            }
            break;
            
        case CMD_QUERY_STATUS:
            send_heartbeat();
            break;
            
        case CMD_SET_ADDR:
            if (data_len >= 1) {
                // 设置新地址 (需要保存到 EEPROM)
                node_status.node_id = data[3];
            }
            break;
    }
}

// ==================== 延时函数 ====================
void delay_ms(uint32_t ms) {
    uint32_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 2400; j++);
    }
}

uint32_t millis() {
    // 使用定时器实现毫秒计时
    static uint32_t count = 0;
    return count++;
}
