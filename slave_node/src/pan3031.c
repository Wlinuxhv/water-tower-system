/*
 * PAN3031 LoRa 驱动 - 极简版
 * 适用于 SDCC 8051 编译，移除 64 位运算
 */

#include <8051.h>

// PAN3031 引脚定义 (必须与 main.c 一致)
__sbit __at (0xA3) PAN3031_CS;    // P2.3
__sbit __at (0xA2) PAN3031_MOSI;  // P2.2
__sbit __at (0xA1) PAN3031_MISO;  // P2.1
__sbit __at (0xA0) PAN3031_SCK;   // P2.0

#include "pan3031.h"

/**
 * 微秒级延时
 */
void delay_us(unsigned int us) {
    while (us--) {
        __asm nop __endasm;
        __asm nop __endasm;
    }
}

/**
 * 毫秒级延时
 */
void delay_ms(unsigned int ms) {
    unsigned int i;
    while (ms--) {
        for (i = 0; i < 200; i++);
    }
}

/**
 * 毫秒计时器 (简化版，由主程序调用递增)
 */
static unsigned long g_millis = 0;

void millis_increment(void) {
    g_millis++;
}

unsigned long millis(void) {
    return g_millis;
}

/**
 * 写寄存器 (软件 SPI)
 */
void pan3031_write_reg(unsigned char addr, unsigned char value) {
    unsigned char i;
    
    PAN3031_CS = 0;
    
    // 发送地址 (写模式，最高位为 1)
    for (i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (addr >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    // 发送数据
    for (i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (value >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    PAN3031_CS = 1;
}

/**
 * 读寄存器 (软件 SPI)
 */
unsigned char pan3031_read_reg(unsigned char addr) {
    unsigned char value = 0;
    unsigned char i;
    
    PAN3031_CS = 0;
    
    // 发送地址 (读模式，最高位为 0)
    for (i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (addr >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    // 读取数据
    for (i = 0; i < 8; i++) {
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
    PAN3031_CS = 1;
    delay_ms(10);
    
    // 进入待机模式
    pan3031_write_reg(0x01, 0x01);
    delay_ms(10);
}

/**
 * 设置频率 (Hz) - 简化版，只支持 434MHz
 */
void pan3031_set_freq(unsigned long freq) {
    // 434MHz 对应的 FRF 值
    // FRF = 434000000 * 2^19 / 32000000 = 71180288 = 0x043D4000
    // 简化：直接写入预计算的值
    pan3031_write_reg(0x06, 0x6C);  // REG_FRF_MSB (434MHz)
    pan3031_write_reg(0x07, 0x80);  // REG_FRF_MID
    pan3031_write_reg(0x08, 0x00);  // REG_FRF_LSB
}

/**
 * 设置扩频因子 (SF7-SF12)
 */
void pan3031_set_sf(unsigned char sf) {
    unsigned char config2;
    
    if (sf < 7) sf = 7;
    if (sf > 12) sf = 12;
    
    config2 = pan3031_read_reg(0x1E);
    config2 = (config2 & 0x0F) | ((sf << 4) & 0xF0);
    pan3031_write_reg(0x1E, config2);
}

/**
 * 设置带宽 (Hz) - 简化版，只支持 125kHz
 */
void pan3031_set_bw(unsigned long bw) {
    // 125kHz 对应 BW=7
    unsigned char config1 = pan3031_read_reg(0x1D);
    config1 = (config1 & 0x0F) | (7 << 4);
    pan3031_write_reg(0x1D, config1);
}

/**
 * 设置发射功率 (dBm)
 */
void pan3031_set_power(unsigned char power) {
    unsigned char pa_config;
    
    if (power > 20) power = 20;
    if (power < 2) power = 2;
    
    pa_config = 0x80 | (power - 2);
    pan3031_write_reg(0x09, pa_config);
}

/**
 * 发送数据
 */
void pan3031_send(unsigned char *data, unsigned char len) {
    unsigned char i, j;
    
    // 进入待机模式
    pan3031_write_reg(0x01, 0x01);
    
    // 设置 FIFO 写指针
    pan3031_write_reg(0x0D, 0x00);
    pan3031_write_reg(0x0E, 0x00);
    
    // 写入数据到 FIFO
    PAN3031_CS = 0;
    pan3031_write_reg(0x00, 0x00);
    PAN3031_CS = 1;
    
    PAN3031_CS = 0;
    for (i = 0; i < len; i++) {
        for (j = 0; j < 8; j++) {
            PAN3031_SCK = 0;
            PAN3031_MOSI = (data[i] >> (7 - j)) & 0x01;
            PAN3031_SCK = 1;
        }
    }
    PAN3031_CS = 1;
    
    // 设置 payload 长度
    pan3031_write_reg(0x22, len);
    
    // 进入 TX 模式
    pan3031_write_reg(0x01, 0x03);
    
    // 等待发送完成
    delay_ms(10);
    
    // 返回待机模式
    pan3031_write_reg(0x01, 0x01);
}

/**
 * 接收数据 - 简化版，始终返回无数据
 */
unsigned char pan3031_receive(unsigned char *data, unsigned char *len) {
    // 简化实现，不实际接收
    (void)data;
    (void)len;
    return 0;
}

/**
 * 进入睡眠模式
 */
void pan3031_sleep(void) {
    pan3031_write_reg(0x01, 0x00);
}
