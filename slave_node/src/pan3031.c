/*
 * PAN3031 LoRa 驱动 - STC32G 实现
 */

#include "pan3031.h"

// 全局变量
static uint32_t g_millis_count = 0;

/**
 * 微秒级延时
 */
void delay_us(uint16_t us) {
    while (us--) {
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
    }
}

/**
 * 毫秒级延时
 */
void delay_ms(uint16_t ms) {
    while (ms--) {
        delay_us(1000);
    }
}

/**
 * 毫秒计时器
 */
uint32_t millis(void) {
    // 简单实现，实际项目中应使用定时器中断
    static uint32_t count = 0;
    return count++;
}

/**
 * 写寄存器
 */
void pan3031_write_reg(uint8_t addr, uint8_t value) {
    PAN3031_CS = 0;
    
    // 发送地址 (写模式，最高位为 1)
    for (uint8_t i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (addr >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    // 发送数据
    for (uint8_t i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (value >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    PAN3031_CS = 1;
}

/**
 * 读寄存器
 */
uint8_t pan3031_read_reg(uint8_t addr) {
    uint8_t value = 0;
    
    PAN3031_CS = 0;
    
    // 发送地址 (读模式，最高位为 0)
    for (uint8_t i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (addr >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    // 读取数据
    for (uint8_t i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_SCK = 1;
        value |= (PAN3031_MISO << (7 - i));
    }
    
    PAN3031_CS = 1;
    
    return value;
}

/**
 * 初始化 PAN3031
 */
void pan3031_init(void) {
    // 配置 GPIO
    // P2.0-P2.3 推挽输出
    P2M0 |= 0x0F;
    P2M1 &= ~0x0F;
    
    // P3.2 输入 (IRQ)
    P3M0 &= ~0x04;
    P3M1 &= ~0x04;
    
    // 复位 PAN3031
    PAN3031_CS = 1;
    delay_ms(10);
    
    // 读取版本寄存器验证通信
    uint8_t version = pan3031_read_reg(0x42);
    if (version == 0x12) {
        // SX1276/SX1278 版本
    }
    
    // 进入待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
    delay_ms(10);
}

/**
 * 设置频率 (Hz)
 */
void pan3031_set_freq(uint32_t freq) {
    uint64_t frf = ((uint64_t)freq << 19) / 32000000;
    
    pan3031_write_reg(REG_FRF_MSB, (frf >> 16) & 0xFF);
    pan3031_write_reg(REG_FRF_MID, (frf >> 8) & 0xFF);
    pan3031_write_reg(REG_FRF_LSB, frf & 0xFF);
}

/**
 * 设置扩频因子 (SF7-SF12)
 */
void pan3031_set_sf(uint8_t sf) {
    if (sf < 7) sf = 7;
    if (sf > 12) sf = 12;
    
    uint8_t config2 = pan3031_read_reg(REG_MODEM_CONFIG2);
    config2 = (config2 & 0x0F) | ((sf << 4) & 0xF0);
    pan3031_write_reg(REG_MODEM_CONFIG2, config2);
}

/**
 * 设置带宽 (Hz)
 */
void pan3031_set_bw(uint32_t bw) {
    uint8_t bw_code;
    
    if (bw <= 7800) bw_code = 0;
    else if (bw <= 10400) bw_code = 1;
    else if (bw <= 15600) bw_code = 2;
    else if (bw <= 20800) bw_code = 3;
    else if (bw <= 31250) bw_code = 4;
    else if (bw <= 41700) bw_code = 5;
    else if (bw <= 62500) bw_code = 6;
    else if (bw <= 125000) bw_code = 7;
    else if (bw <= 250000) bw_code = 8;
    else bw_code = 9;
    
    uint8_t config1 = pan3031_read_reg(REG_MODEM_CONFIG1);
    config1 = (config1 & 0x0F) | (bw_code << 4);
    pan3031_write_reg(REG_MODEM_CONFIG1, config1);
}

/**
 * 设置发射功率 (dBm)
 */
void pan3031_set_power(uint8_t power) {
    if (power > 20) power = 20;
    if (power < 2) power = 2;
    
    uint8_t pa_config = 0x80 | (power - 2);
    pan3031_write_reg(REG_PA_CONFIG, pa_config);
}

/**
 * 发送数据
 */
void pan3031_send(uint8_t *data, uint8_t len) {
    // 进入待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
    
    // 设置 FIFO 写指针
    pan3031_write_reg(REG_FIFO_ADDR_PTR, 0x00);
    pan3031_write_reg(REG_FIFO_TX_BASE, 0x00);
    
    // 写入数据到 FIFO
    PAN3031_CS = 0;
    pan3031_write_reg(REG_FIFO, 0x00);  // FIFO 地址
    PAN3031_CS = 1;
    
    PAN3031_CS = 0;
    // 发送 FIFO 写命令
    for (uint8_t i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = 0x80;  // 写 FIFO
        PAN3031_SCK = 1;
    }
    
    // 发送数据
    for (uint8_t i = 0; i < len; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            PAN3031_SCK = 0;
            PAN3031_MOSI = (data[i] >> (7 - j)) & 0x01;
            PAN3031_SCK = 1;
        }
    }
    PAN3031_CS = 1;
    
    // 设置 payload 长度
    pan3031_write_reg(REG_PAYLOAD_LEN, len);
    
    // 进入 TX 模式
    pan3031_write_reg(REG_OP_MODE, MODE_TX);
    
    // 等待发送完成
    delay_ms(10);
    
    // 返回待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
}

/**
 * 接收数据
 * 返回：1=接收到数据，0=无数据
 */
uint8_t pan3031_receive(uint8_t *data, uint8_t *len) {
    // 检查 IRQ
    if (PAN3031_IRQ == 1) {
        return 0;  // 无数据
    }
    
    // 进入 RX 连续模式
    pan3031_write_reg(REG_OP_MODE, MODE_RXCONT);
    
    // 等待接收完成 (简单超时)
    uint16_t timeout = 1000;
    while (PAN3031_IRQ == 1 && timeout--) {
        delay_us(100);
    }
    
    if (timeout == 0) {
        pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
        return 0;
    }
    
    // 读取接收字节数
    *len = pan3031_read_reg(REG_RX_NB_BYTES);
    if (*len > 32) *len = 32;
    
    // 读取 FIFO 地址
    uint8_t fifo_addr = pan3031_read_reg(REG_FIFO_RX_ADDR);
    pan3031_write_reg(REG_FIFO_ADDR_PTR, fifo_addr);
    
    // 读取数据
    PAN3031_CS = 0;
    pan3031_write_reg(REG_FIFO, 0x00);
    PAN3031_CS = 1;
    
    PAN3031_CS = 0;
    for (uint8_t i = 0; i < *len; i++) {
        data[i] = 0;
        for (uint8_t j = 0; j < 8; j++) {
            PAN3031_SCK = 0;
            PAN3031_SCK = 1;
            data[i] |= (PAN3031_MISO << (7 - j));
        }
    }
    PAN3031_CS = 1;
    
    // 清除 IRQ
    pan3031_write_reg(REG_IRQ_FLAGS, 0xFF);
    
    // 返回待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
    
    return 1;
}

/**
 * 进入睡眠模式
 */
void pan3031_sleep(void) {
    pan3031_write_reg(REG_OP_MODE, MODE_SLEEP);
}
