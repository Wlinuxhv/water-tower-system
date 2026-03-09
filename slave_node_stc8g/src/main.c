/*
 * 水塔监控从机节点 - STC8G1K08 传感器专用版
 * 
 * 架构变更：
 * - ESP8266 主机控制水泵继电器
 * - STC8G1K08 从机只负责传感器数据采集
 * 
 * 功能：
 * 1. 读取液位传感器 (ADC)
 * 2. 检测缺水
 * 3. 通过 PAN3031 与主机通信
 * 4. 接收主机命令 (只读，不执行水泵控制)
 */

#include <8051.h>
#include "pan3031.h"
#include "slave_config.h"

// ==================== 引脚定义 ====================
// PAN3031 LoRa 通信
sbit PAN3031_CS   = P2^3;
sbit PAN3031_MOSI = P2^2;
sbit PAN3031_MISO = P2^1;
sbit PAN3031_SCK  = P2^0;

// 传感器输入 (只读)
sbit WATER_LOW_DET = P3^4;  // 缺水检测
// 注意：水泵继电器由 ESP8266 主机控制，STC8G 不控制

// ==================== 全局变量 ====================
volatile unsigned char node_id = NODE_ID;
volatile unsigned char water_level = 0;
volatile unsigned char well_water_ok = 1;
volatile unsigned long last_send = 0;

// ==================== 函数声明 ====================
void system_init(void);
unsigned char read_water_level(void);
unsigned char check_well_water(void);
void send_sensor_data(void);
void handle_host_command(void);
void delay_ms(unsigned int ms);
unsigned long millis(void);

// ==================== 主函数 ====================
void main(void) {
    system_init();
    
    // 发送上电心跳
    send_sensor_data();
    
    while (1) {
        // 读取水位传感器
        water_level = read_water_level();
        
        // 检查井水是否缺水
        well_water_ok = check_well_water();
        
        // 定期发送传感器数据 (每 5 秒)
        if (millis() - last_send > 5000) {
            send_sensor_data();
        }
        
        // 处理主机命令 (非阻塞)
        handle_host_command();
        
        delay_ms(100);
    }
}

// ==================== 系统初始化 ====================
void system_init(void) {
    // GPIO 初始化 (全部设为输入)
    P0 = 0xFF;
    P1 = 0xFF;
    P2 = 0xFF;
    P3 = 0xFF;
    
    // PAN3031 LoRa 初始化
    pan3031_init();
    pan3031_set_freq(434000000UL);  // 434MHz
    pan3031_set_sf(7);              // SF7
    pan3031_set_bw(125000UL);       // 125kHz
    pan3031_set_power(20);          // 20dBm
    
    // 串口调试 (可选)
    // SCON = 0x50;  // 串口模式 1
    // TMOD |= 0x20; // 定时器 1 模式 2
    // TH1 = 0xFD;   // 9600bps @ 11.0592MHz
    // TR1 = 1;      // 启动定时器 1
}

// ==================== 传感器读取 ====================
/**
 * 读取水位传感器
 * 
 * 实际硬件连接:
 * - 液位传感器 -> ADC 引脚 (P1.0/AIN0)
 * - 返回 0-100 百分比
 * 
 * 简化版：使用模拟值演示
 */
unsigned char read_water_level(void) {
    // 实际项目中使用 ADC 读取
    // ADC_CONTR = 0x80;  // 开启 ADC
    // ADC_RES = 0;       // 清除结果
    // ... ADC 转换代码 ...
    
    // 简化版：模拟水位变化 (演示用)
    static unsigned char sim_level = 50;
    sim_level += 2;
    if (sim_level > 100) sim_level = 0;
    
    return sim_level;
}

/**
 * 检查井水是否缺水
 * 
 * 硬件连接:
 * - 缺水传感器 -> P3.4 (数字输入)
 * - 高电平：有水 OK
 * - 低电平：缺水 NG
 */
unsigned char check_well_water(void) {
    return (WATER_LOW_DET == 1) ? 1 : 0;
}

// ==================== 通信函数 ====================
/**
 * 发送传感器数据到主机
 * 
 * 数据格式:
 * [NodeID][CMD_SENSOR][Len][WaterLevel][WellWaterOK]
 */
void send_sensor_data(void) {
    unsigned char tx_data[5];
    
    tx_data[0] = node_id;           // 从机 ID
    tx_data[1] = CMD_SENSOR_DATA;   // 传感器数据命令
    tx_data[2] = 2;                 // 数据长度
    tx_data[3] = water_level;       // 水位 0-100%
    tx_data[4] = well_water_ok ? 1 : 0;  // 井水状态
    
    pan3031_send(tx_data, 5);
    last_send = millis();
    
    // 调试输出
    // printf("Send: ID=%d Level=%d Well=%d\n", node_id, water_level, well_water_ok);
}

/**
 * 处理主机命令
 * 
 * 支持命令:
 * - CMD_READ_SENSOR: 读取传感器 (立即响应)
 * - CMD_PUMP_CTRL: 水泵控制 (忽略，由 ESP8266 直接控制)
 * - CMD_HEARTBEAT: 心跳请求
 */
void handle_host_command(void) {
    unsigned char rx_data[8];
    unsigned char len = pan3031_receive(rx_data, 8);
    
    if (len < 4) return;  // 数据太短
    
    // 验证目标地址
    if (rx_data[1] != node_id) return;  // 不是给我的
    
    unsigned char cmd = rx_data[2];
    
    switch (cmd) {
        case CMD_READ_SENSOR:
            // 立即发送传感器数据
            send_sensor_data();
            break;
            
        case CMD_PUMP_CTRL:
            // 忽略水泵控制命令 (ESP8266 直接控制继电器)
            // 只记录状态，不执行
            break;
            
        case CMD_HEARTBEAT:
            // 心跳响应
            send_sensor_data();
            break;
            
        default:
            // 未知命令
            break;
    }
}

// ==================== 工具函数 ====================
/**
 * 毫秒级延时
 */
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 1200; j++);  // @11.0592MHz
    }
}

/**
 * 系统运行时间 (毫秒)
 */
unsigned long millis(void) {
    static unsigned long count = 0;
    // 实际应该用定时器中断
    return count++;
}
