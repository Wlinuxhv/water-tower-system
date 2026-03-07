/*
 * PAN3031 LoRa 驱动 - 函数声明
 */

#ifndef PAN3031_H
#define PAN3031_H

#include <stdint.h>

// 命令字定义
#define CMD_HEARTBEAT   0x01
#define CMD_QUERY       0x02
#define CMD_PUMP_CTRL   0x10
#define CMD_SET_AUTO    0x20
#define CMD_SET_MANUAL  0x21
#define CMD_ALARM       0xFF

// 函数声明
void pan3031_init(void);
void pan3031_set_freq(unsigned long freq);
void pan3031_set_sf(unsigned char sf);
void pan3031_set_bw(unsigned long bw);
void pan3031_set_power(unsigned char power);
void pan3031_send(unsigned char *data, unsigned char len);
unsigned char pan3031_receive(unsigned char *data, unsigned char *len);
void pan3031_sleep(void);

// 延时函数
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);
unsigned long millis(void);
void millis_increment(void);

#endif // PAN3031_H
