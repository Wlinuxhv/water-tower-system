/*
 * PAN3031 LoRa 驱动 - STC8G1K08 简化版
 */

#include <8051.h>
#include "pan3031.h"

// 引脚定义
__sbit __at(0xA3) PAN3031_CS;
__sbit __at(0xA2) PAN3031_MOSI;
__sbit __at(0xA1) PAN3031_MISO;
__sbit __at(0xA0) PAN3031_SCK;

// 全局变量
static unsigned long g_millis = 0;

// 微秒延时
void delay_us(unsigned int us) {
    while (us--) {
        __asm nop __endasm;
        __asm nop __endasm;
    }
}

// 毫秒延时
void delay_ms(unsigned int ms) {
    unsigned int i;
    while (ms--) {
        for (i = 0; i < 200; i++);
    }
}

// 毫秒计时
unsigned long millis(void) {
    return g_millis++;
}

// 写寄存器
void pan3031_write_reg(unsigned char addr, unsigned char value) {
    unsigned char i;
    
    PAN3031_CS = 0;
    
    for (i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (addr >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    for (i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (value >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    PAN3031_CS = 1;
}

// 读寄存器
unsigned char pan3031_read_reg(unsigned char addr) {
    unsigned char value = 0;
    unsigned char i;
    
    PAN3031_CS = 0;
    
    for (i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_MOSI = (addr >> (7 - i)) & 0x01;
        PAN3031_SCK = 1;
    }
    
    for (i = 0; i < 8; i++) {
        PAN3031_SCK = 0;
        PAN3031_SCK = 1;
        value |= (PAN3031_MISO << (7 - i));
    }
    
    PAN3031_CS = 1;
    
    return value;
}

// 初始化
void pan3031_init(void) {
    PAN3031_CS = 1;
    delay_ms(10);
    
    pan3031_write_reg(0x01, 0x01);  // 待机模式
    delay_ms(10);
}

// 设置频率 (434MHz)
void pan3031_set_freq(unsigned long freq) {
    (void)freq;  // 忽略参数，固定 434MHz
    pan3031_write_reg(0x06, 0x6C);
    pan3031_write_reg(0x07, 0x80);
    pan3031_write_reg(0x08, 0x00);
}

// 设置扩频因子
void pan3031_set_sf(unsigned char sf) {
    unsigned char config2;
    
    if (sf < 7) sf = 7;
    if (sf > 12) sf = 12;
    
    config2 = pan3031_read_reg(0x1E);
    config2 = (config2 & 0x0F) | ((sf << 4) & 0xF0);
    pan3031_write_reg(0x1E, config2);
}

// 设置带宽 (125kHz)
void pan3031_set_bw(unsigned long bw) {
    (void)bw;  // 忽略参数，固定 125kHz
    unsigned char config1 = pan3031_read_reg(0x1D);
    config1 = (config1 & 0x0F) | (7 << 4);
    pan3031_write_reg(0x1D, config1);
}

// 设置功率
void pan3031_set_power(unsigned char power) {
    unsigned char pa_config;
    
    if (power > 20) power = 20;
    if (power < 2) power = 2;
    
    pa_config = 0x80 | (power - 2);
    pan3031_write_reg(0x09, pa_config);
}

// 发送数据
void pan3031_send(unsigned char *data, unsigned char len) {
    unsigned char i, j;
    
    pan3031_write_reg(0x01, 0x01);  // 待机
    pan3031_write_reg(0x0D, 0x00);  // FIFO 指针
    pan3031_write_reg(0x0E, 0x00);
    
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
    
    pan3031_write_reg(0x22, len);
    pan3031_write_reg(0x01, 0x03);  // TX 模式
    delay_ms(10);
    pan3031_write_reg(0x01, 0x01);  // 待机
}

// 接收数据 (简化版)
unsigned char pan3031_receive(unsigned char *data, unsigned char *len) {
    (void)data;
    (void)len;
    return 0;  // 不实现接收
}

// 睡眠模式
void pan3031_sleep(void) {
    pan3031_write_reg(0x01, 0x00);
}
