/*
 * 水塔监控从机节点 - STC8G1K08 低功耗版
 * 
 * 功能:
 * 1. 读取液位传感器 (ADC)
 * 2. 检测缺水
 * 3. 控制水泵继电器
 * 4. 通过 PAN3031 与主机通信
 * 5. 超低功耗睡眠模式
 * 
 * 硬件:
 * - MCU: STC8G1K08 (8KB Flash, 512B RAM)
 * - LoRa: PAN3031 (SPI)
 * - 传感器：4-20mA 液位变送器 (ADC)
 * 
 * 功耗:
 * - 工作：~5mA
 * - 睡眠：<10μA
 * - 唤醒间隔：5 秒
 */

#include <STC8G1K08.h>
#include "pan3031.h"
#include "slave_config.h"

// ==================== 引脚定义 ====================
// PAN3031 (SPI)
sfr PAN3031_CS   = 0xB0;  // P2.3
sfr PAN3031_MOSI = 0xB0;  // P2.2
sfr PAN3031_MISO = 0xB0;  // P2.1
sfr PAN3031_SCK  = 0xB0;  // P2.0
sbit PAN3031_CS_PIN   = P2^3;
sbit PAN3031_MOSI_PIN = P2^2;
sbit PAN3031_MISO_PIN = P2^1;
sbit PAN3031_SCK_PIN  = P2^0;

// 水泵继电器
sbit PUMP_RELAY = P3^3;

// 缺水检测 (数字输入，高电平有水)
sbit WATER_LOW_DET = P3^4;

// ADC 输入 (液位传感器)
// STC8G1K08 ADC 在 P1.0

// ==================== 全局变量 ====================
volatile uint8_t g_node_id = NODE_ID;
volatile uint8_t g_water_level = 0;
volatile bit g_pump_on = 0;
volatile bit g_well_water_ok = 1;
volatile uint32_t g_last_send = 0;
volatile uint16_t g_timer_count = 0;

// ==================== 函数声明 ====================
void system_init(void);
void adc_init(void);
uint8_t read_water_level(void);
bit check_well_water(void);
void pump_control(bit on);
void send_heartbeat(void);
void handle_command(uint8_t *data, uint8_t len);
void enter_sleep_mode(void);
void timer0_init(void);

// ==================== 主函数 ====================
void main(void) {
    system_init();
    
    // 上电后发送心跳
    send_heartbeat();
    
    while (1) {
        // 读取传感器
        g_water_level = read_water_level();
        g_well_water_ok = check_well_water();
        
        // 处理接收到的命令
        uint8_t rx_data[16];
        uint8_t rx_len;
        if (pan3031_receive(rx_data, &rx_len)) {
            handle_command(rx_data, rx_len);
        }
        
        // 定期发送心跳 (每 5 秒)
        if (g_timer_count - g_last_send > SEND_INTERVAL) {
            send_heartbeat();
        }
        
        // 进入睡眠模式 (低功耗)
        enter_sleep_mode();
    }
}

// ==================== 系统初始化 ====================
void system_init(void) {
    // 关闭所有数字端口的高阻输入，降低功耗
    P0M0 = 0x00; P0M1 = 0x00;  // P0 准双向
    P1M0 = 0x00; P1M1 = 0x00;  // P1 准双向
    P2M0 = 0x08; P2M1 = 0x00;  // P2.3 推挽输出 (CS)
    P3M0 = 0x08; P3M1 = 0x00;  // P3.3 推挽输出 (继电器)
    
    // 初始关闭水泵
    pump_control(0);
    
    // PAN3031 初始化
    pan3031_init();
    pan3031_set_freq(PAN3031_FREQ);
    pan3031_set_sf(PAN3031_SF);
    pan3031_set_bw(PAN3031_BW);
    pan3031_set_power(PAN3031_PWR);
    
    // ADC 初始化
    adc_init();
    
    // 定时器 0 初始化 (用于计时)
    timer0_init();
    
    // 开启总中断
    EA = 1;
}

// ==================== ADC 初始化 ====================
void adc_init(void) {
    // STC8G1K08 ADC 配置
    // ADC 时钟 = 系统时钟 / 分频
    // 使用内部 2.56V 参考电压
    
    ADC_CONTR = 0x80;  // 开启 ADC 电源
    ADC_RES = 0x00;
    ADC_RESL = 0x00;
}

// ==================== 读取水位 ====================
uint8_t read_water_level(void) {
    uint16_t adc_value;
    
    // 启动 ADC 转换 (通道 0 = P1.0)
    ADC_CONTR = 0x88;  // 开启 ADC，选择通道 0
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    
    // 等待转换完成
    while (!(ADC_CONTR & 0x10));
    
    // 读取结果 (10 位精度)
    adc_value = ((uint16_t)ADC_RES << 2) | (ADC_RESL >> 6);
    
    // 清除完成标志
    ADC_CONTR &= ~0x10;
    
    // 转换为百分比 (0-100%)
    // 4-20mA 传感器：0% = 4mA, 100% = 20mA
    // ADC: 0 = 0V, 1023 = 2.56V
    // 假设传感器输出 0.5V-2.5V 对应 0-100%
    
    if (adc_value < 200) adc_value = 200;  // 下限
    if (adc_value > 1000) adc_value = 1000; // 上限
    
    return (uint8_t)((adc_value - 200) * 100 / 800);
}

// ==================== 检查缺水 ====================
bit check_well_water(void) {
    return WATER_LOW_DET;  // 高电平表示有水
}

// ==================== 水泵控制 ====================
void pump_control(bit on) {
    PUMP_RELAY = on;
    g_pump_on = on;
}

// ==================== 发送心跳 ====================
void send_heartbeat(void) {
    uint8_t tx_data[6];
    
    tx_data[0] = 0x00;           // 主机地址 (目标)
    tx_data[1] = g_node_id;      // 源地址
    tx_data[2] = CMD_HEARTBEAT;  // 命令字
    tx_data[3] = 3;              // 数据长度
    tx_data[4] = g_water_level;  // 水位
    tx_data[5] = (g_well_water_ok << 1) | g_pump_on;  // 状态位
    
    pan3031_send(tx_data, 6);
    g_last_send = g_timer_count;
}

// ==================== 处理命令 ====================
void handle_command(uint8_t *data, uint8_t len) {
    if (len < 4) return;
    
    uint8_t src_id = data[0];
    uint8_t cmd = data[2];
    uint8_t data_len = data[3];
    
    // 只响应主机 (地址 0x00)
    if (src_id != 0x00) return;
    
    switch (cmd) {
        case CMD_PUMP_CTRL:
            if (data_len >= 1 && len >= 5) {
                pump_control(data[4] ? 1 : 0);
            }
            break;
            
        case CMD_QUERY:
            // 立即回复状态
            send_heartbeat();
            break;
            
        case CMD_SET_AUTO:
            // 切换到自动模式 (本地逻辑)
            break;
            
        case CMD_SET_MANUAL:
            // 切换到手动模式
            break;
    }
}

// ==================== 进入睡眠模式 ====================
void enter_sleep_mode(void) {
    // 配置外部中断唤醒 (PAN3031 IRQ)
    // 当 PAN3031 接收到数据时，IRQ 引脚拉低唤醒 MCU
    
    // 开启 INT0 中断 (P3.2)
    IT0 = 1;   // 下降沿触发
    EX0 = 1;   // 允许 INT0 中断
    
    // 进入空闲模式 (IDLE)
    // STC8G1K08: PCON |= 0x01
    PCON |= 0x01;
    
    // 唤醒后继续执行
    // 关闭中断
    EX0 = 0;
}

// ==================== 定时器 0 中断 (计时) ====================
void timer0_isr(void) interrupt 1 {
    // 12MHz 系统时钟，12T 模式
    // 定时 1ms
    TH0 = (65536 - 1000) / 256;
    TL0 = (65536 - 1000) % 256;
    
    g_timer_count++;
}

// ==================== 定时器 0 初始化 ====================
void timer0_init(void) {
    TMOD &= ~0x03;  // 定时器 0，模式 1 (16 位)
    TMOD |= 0x01;
    
    // 1ms 定时
    TH0 = (65536 - 1000) / 256;
    TL0 = (65536 - 1000) % 256;
    
    ET0 = 1;  // 允许定时器 0 中断
    TR0 = 1;  // 启动定时器 0
}

// ==================== 外部中断 0 (唤醒) ====================
void ext_int0_isr(void) interrupt 0 {
    // 唤醒后处理，什么也不做
    // 主循环会检查接收缓冲区
}
