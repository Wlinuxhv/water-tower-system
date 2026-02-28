/*
 * PAN3031 LoRa 驱动 - ESP8266 版
 */

#include "pan3031.h"
#include <SPI.h>

// 引脚
static uint8_t PIN_CS, PIN_MOSI, PIN_MISO, PIN_SCK, PIN_IRQ;

// ==================== 初始化 ====================
void pan3031_init(uint8_t cs, uint8_t mosi, uint8_t miso, uint8_t sck, uint8_t irq) {
    PIN_CS = cs;
    PIN_MOSI = mosi;
    PIN_MISO = miso;
    PIN_SCK = sck;
    PIN_IRQ = irq;
    
    // 配置引脚
    pinMode(PIN_CS, OUTPUT);
    pinMode(PIN_IRQ, INPUT);
    digitalWrite(PIN_CS, HIGH);
    
    // 初始化 SPI
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    
    // 复位 PAN3031 (延时等待稳定)
    delay(10);
    
    // 读取版本验证
    uint8_t version = pan3031_read_reg(REG_SYNC_WORD);
    Serial.print("PAN3031 版本：0x");
    Serial.println(version, HEX);
    
    // 进入待机模式
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
}

// ==================== SPI 寄存器操作 ====================
void pan3031_write_reg(uint8_t addr, uint8_t value) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(addr | 0x80);  // 写操作
    SPI.transfer(value);
    digitalWrite(PIN_CS, HIGH);
}

uint8_t pan3031_read_reg(uint8_t addr) {
    uint8_t value;
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(addr & ~0x80);  // 读操作
    value = SPI.transfer(0x00);
    digitalWrite(PIN_CS, HIGH);
    return value;
}

// ==================== 频率配置 ====================
void pan3031_set_freq(uint32_t freq) {
    uint32_t frf = ((uint32_t)freq << 19) / 32000000;
    
    pan3031_write_reg(REG_FRF_MSB, (frf >> 16) & 0xFF);
    pan3031_write_reg(REG_FRF_MID, (frf >> 8) & 0xFF);
    pan3031_write_reg(REG_FRF_LSB, frf & 0xFF);
}

// ==================== 扩频因子 ====================
void pan3031_set_sf(uint8_t sf) {
    uint8_t config2 = pan3031_read_reg(REG_MODEM_CONFIG2);
    config2 = (config2 & 0x0F) | ((sf << 4) & 0xF0);
    pan3031_write_reg(REG_MODEM_CONFIG2, config2);
}

// ==================== 带宽配置 ====================
void pan3031_set_bw(uint32_t bw) {
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
    else bw_val = 9;
    
    uint8_t config1 = pan3031_read_reg(REG_MODEM_CONFIG1);
    config1 = (config1 & 0x0F) | (bw_val << 4);
    pan3031_write_reg(REG_MODEM_CONFIG1, config1);
}

// ==================== 功率配置 ====================
void pan3031_set_power(uint8_t power) {
    if (power < 2) power = 2;
    if (power > 17) power = 17;
    
    uint8_t pa_config = 0x80 | (power - 2);
    pan3031_write_reg(REG_PA_CONFIG, pa_config);
}

// ==================== 发送数据 ====================
void pan3031_send(uint8_t *data, uint8_t len) {
    // 进入待机
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
    
    // 设置 FIFO 指针
    pan3031_write_reg(REG_FIFO_ADDR_PTR, 0x00);
    pan3031_write_reg(REG_FIFO_TX_BASE, 0x00);
    
    // 写入数据
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(REG_FIFO | 0x80);
    for (uint8_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    digitalWrite(PIN_CS, HIGH);
    
    // 设置长度
    pan3031_write_reg(REG_PAYLOAD_LEN, len);
    
    // 发送
    pan3031_write_reg(REG_OP_MODE, MODE_TX);
    
    // 等待完成
    while ((pan3031_read_reg(REG_IRQ_FLAGS) & 0x08) == 0);
    
    // 清除中断
    pan3031_write_reg(REG_IRQ_FLAGS, 0xFF);
    
    // 返回待机
    pan3031_write_reg(REG_OP_MODE, MODE_STDBY);
}

// ==================== 接收数据 ====================
bool pan3031_receive(uint8_t *data, uint8_t *len) {
    // 连续接收
    pan3031_write_reg(REG_OP_MODE, MODE_RXCONT);
    
    // 检查中断
    uint8_t irq_flags = pan3031_read_reg(REG_IRQ_FLAGS);
    
    if (irq_flags & 0x40) {  // RxDone
        uint8_t rx_len = pan3031_read_reg(REG_RX_NB_BYTES);
        if (rx_len > 32) rx_len = 32;
        *len = rx_len;
        
        // 读取 FIFO
        uint8_t fifo_addr = pan3031_read_reg(REG_FIFO_RX_ADDR);
        pan3031_write_reg(REG_FIFO_ADDR_PTR, fifo_addr);
        
        digitalWrite(PIN_CS, LOW);
        SPI.transfer(REG_FIFO | 0x80);
        for (uint8_t i = 0; i < rx_len; i++) {
            data[i] = SPI.transfer(0x00);
        }
        digitalWrite(PIN_CS, HIGH);
        
        // 清除中断
        pan3031_write_reg(REG_IRQ_FLAGS, 0xFF);
        
        return true;
    }
    
    return false;
}

// ==================== 睡眠模式 ====================
void pan3031_sleep(void) {
    pan3031_write_reg(REG_OP_MODE, MODE_SLEEP);
}
