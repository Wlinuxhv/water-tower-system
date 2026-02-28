/*
 * PAN3031 LoRa 驱动 - STC8G1K08 版
 */

#include "pan3031.h"

// 引脚定义 (已在头文件中定义)
// CS: P2.3, MOSI: P2.2, MISO: P2.1, SCK: P2.0

// ==================== SPI 基础函数 ====================
static void spi_delay(void) {
    _nop_();
    _nop_();
}

static void spi_init(void) {
    // 配置 SPI 引脚
    // SCK: P2.0 (输出)
    // MOSI: P2.2 (输出)
    // MISO: P2.1 (输入)
    // CS: P2.3 (输出)
    
    P2M0 |= 0x0D;  // P2.0, P2.2, P2.3 推挽输出
    P2M1 &= ~0x0D;
    
    PAN3031_CS_PIN = 1;  // 初始 CS 高电平
}

static uint8_t spi_transfer(uint8_t data) {
    uint8_t i, result = 0;
    
    for (i = 0; i < 8; i++) {
        // 输出 MOSI
        if (data & 0x80)
            PAN3031_MOSI_PIN = 1;
        else
            PAN3031_MOSI_PIN = 0;
        
        data <<= 1;
        
        // SCK 拉高
        PAN3031_SCK_PIN = 1;
        spi_delay();
        
        // 读取 MISO
        result <<= 1;
        if (PAN3031_MISO_PIN)
            result |= 0x01;
        
        // SCK 拉低
        PAN3031_SCK_PIN = 0;
        spi_delay();
    }
    
    return result;
}

// ==================== PAN3031 基础操作 ====================
void pan3031_init(void) {
    spi_init();
    
    // 复位 PAN3031 (如果有复位引脚)
    // 这里假设通过电源管理
    
    // 等待模块稳定
    delay_ms(10);
    
    // 读取版本寄存器验证通信
    uint8_t version = pan3031_read_reg(REG_VERSION);
    // PAN3031 版本应该是 0x12
    
    // 进入待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
}

void pan3031_write_reg(uint8_t addr, uint8_t value) {
    PAN3031_CS_PIN = 0;
    spi_transfer(addr | 0x80);  // 写操作
    spi_transfer(value);
    PAN3031_CS_PIN = 1;
}

uint8_t pan3031_read_reg(uint8_t addr) {
    uint8_t value;
    
    PAN3031_CS_PIN = 0;
    spi_transfer(addr & ~0x80);  // 读操作
    value = spi_transfer(0x00);
    PAN3031_CS_PIN = 1;
    
    return value;
}

// ==================== PAN3031 配置 ====================
void pan3031_set_freq(uint32_t freq) {
    uint32_t frf = ((uint32_t)freq << 19) / 32000000;
    
    pan3031_write_reg(REG_FRF_MSB, (frf >> 16) & 0xFF);
    pan3031_write_reg(REG_FRF_MID, (frf >> 8) & 0xFF);
    pan3031_write_reg(REG_FRF_LSB, frf & 0xFF);
}

void pan3031_set_sf(uint8_t sf) {
    // 设置扩频因子 (6-12)
    uint8_t config2 = pan3031_read_reg(REG_MODEM_CONFIG2);
    config2 = (config2 & 0x0F) | ((sf << 4) & 0xF0);
    pan3031_write_reg(REG_MODEM_CONFIG2, config2);
}

void pan3031_set_bw(uint32_t bw) {
    // 设置带宽
    uint8_t bw_val;
    
    if (bw <= 7800) bw_val = 0;
    else if (bw <= 10400) bw_val = 1;
    else if (bw <= 15600) bw_val = 2;
    else if (bw <= 20800) bw_val = 3;
    else if (bw <= 31250) bw_val = 4;
    else if (bw <= 41700) bw_val = 5;
    else if (bw <= 62500) bw_val = 6;
    else if (bw <= 125000) bw_val = 7;
    else if (bw <= 250000) bw_val = 8;
    else bw_val = 9;  // 500kHz
    
    uint8_t config1 = pan3031_read_reg(REG_MODEM_CONFIG1);
    config1 = (config1 & 0x0F) | (bw_val << 4);
    pan3031_write_reg(REG_MODEM_CONFIG1, config1);
}

void pan3031_set_power(uint8_t power) {
    // 设置发射功率 (2-17 dBm)
    if (power < 2) power = 2;
    if (power > 17) power = 17;
    
    uint8_t pa_config = 0x80 | (power - 2);
    pan3031_write_reg(REG_PA_CONFIG, pa_config);
}

// ==================== 发送数据 ====================
void pan3031_send(uint8_t *data, uint8_t len) {
    uint8_t i;
    
    // 进入待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
    
    // 设置 FIFO 写指针
    pan3031_write_reg(REG_FIFO_ADDR_PTR, 0x00);
    pan3031_write_reg(REG_FIFO_TX_BASE, 0x00);
    
    // 写入数据到 FIFO
    PAN3031_CS_PIN = 0;
    spi_transfer(REG_FIFO | 0x80);
    for (i = 0; i < len; i++) {
        spi_transfer(data[i]);
    }
    PAN3031_CS_PIN = 1;
    
    // 设置 payload 长度
    pan3031_write_reg(REG_PAYLOAD_LEN, len);
    
    // 切换到发送模式
    pan3031_write_reg(REG_OP_MODE, MODE_TX);
    
    // 等待发送完成
    while ((pan3031_read_reg(REG_IRQ_FLAGS) & 0x08) == 0);
    
    // 清除中断标志
    pan3031_write_reg(REG_IRQ_FLAGS, 0xFF);
    
    // 返回待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
}

// ==================== 接收数据 ====================
bit pan3031_receive(uint8_t *data, uint8_t *len) {
    uint8_t i, rx_len;
    
    // 进入连续接收模式
    pan3031_write_reg(REG_OP_MODE, MODE_RXCONT);
    
    // 检查是否有数据
    uint8_t irq_flags = pan3031_read_reg(REG_IRQ_FLAGS);
    
    if (irq_flags & 0x40) {  // RxDone
        // 读取接收字节数
        rx_len = pan3031_read_reg(REG_RX_NB_BYTES);
        
        if (rx_len > 16) rx_len = 16;  // 限制最大长度
        *len = rx_len;
        
        // 读取 FIFO 数据
        uint8_t fifo_addr = pan3031_read_reg(REG_FIFO_RX_ADDR);
        pan3031_write_reg(REG_FIFO_ADDR_PTR, fifo_addr);
        
        PAN3031_CS_PIN = 0;
        spi_transfer(REG_FIFO | 0x80);
        for (i = 0; i < rx_len; i++) {
            data[i] = spi_transfer(0x00);
        }
        PAN3031_CS_PIN = 1;
        
        // 清除中断
        pan3031_write_reg(REG_IRQ_FLAGS, 0xFF);
        
        return 1;  // 接收到数据
    }
    
    return 0;  // 无数据
}

// ==================== 低功耗模式 ====================
void pan3031_sleep(void) {
    // 进入睡眠模式
    pan3031_write_reg(REG_OP_MODE, MODE_SLEEP);
}

void pan3031_wor_enable(void) {
    // 启用 WOR (Watch On Receiver)
    // 定时唤醒接收，降低功耗
    
    // 配置 WOR 周期
    pan3031_write_reg(REG_PLL_HOP, 0x02);  // WOR 周期设置
    
    // 进入 WOR 模式
    pan3031_write_reg(REG_OP_MODE, MODE_RXSINGLE);
}

// ==================== 延时函数 ====================
void delay_ms(uint16_t ms) {
    uint16_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 1200; j++);  // 根据时钟频率调整
    }
}
